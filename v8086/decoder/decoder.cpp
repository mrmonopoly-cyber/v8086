#include "decoder.h"

#include <cassert>
#include <stdio.h>

#include <v8086_definitions.h>

enum ModField{
  MemModeX  =  0b00,
  MemMode8  =  0b01,
  MemMode16 =  0b10,
  RegMode   =  0b11,
};

typedef u32 (*f_instruction_decoder)(
    const u8 first_byte,
    const u8* const mem,
    const u32 size,
    Instruction* out);

static Register regs_dec[][2]
{
  {al, ax},
  {cl, cx},
  {dl, dx},
  {bl, bx},
  {ah, sp},
  {ch, bp},
  {dh, si},
  {bh, di},
};

static f_instruction_decoder decoders[255];

static u32 _decode_invalid(const u8 first_byte, const u8* mem, const u32 size, Instruction* out)
{
  u32 res=0;
  UNUSED(first_byte);
  UNUSED(mem);
  UNUSED(size);

  out->op = Opcode::INVALID;

  return res;
}

static inline u8 _get_w_bit(const u8 byte)
{
  return (byte >> 0) & 0x1;
}

static inline u8 _get_d_bit(const u8 byte)
{
  return (byte >> 1) & 0x1;
}

static inline ModField _get_mod_field(const u8 byte)
{
  return (ModField) (byte >> 6 & 0x3);
}

static inline Register _get_reg_field(const u8 byte, const u8 w)
{
  return regs_dec[(byte >> 3) & 0x7][w];
}

static inline u8 _get_rm_field(const u8 byte)
{
  return (byte >> 0) & 0x7;
}

static inline s8 _mod_rm_to_arg(const u8* mem, u8 size, ModField mod, u8 rm, u8 w, Arg* out)
{
  s8 res = mod;

  switch (rm)
  {
    case 0b000:
      out->reg_reg_disp.r1 = bx;
      out->reg_reg_disp.r2 = si;
      break;
    case 0b001:
      out->reg_reg_disp.r1 = bx;
      out->reg_reg_disp.r2 = di;
      break;
    case 0b010:
      out->reg_reg_disp.r1 = bp;
      out->reg_reg_disp.r2 = si;
      break;
    case 0b011:
      out->reg_reg_disp.r1 = bp;
      out->reg_reg_disp.r2 = di;
      break;
    case 0b100:
      out->reg_disp.r1 = si;
      break;
    case 0b101:
      out->reg_disp.r1 = di;
      break;
    case 0b110:
      switch (mod)
      {
        case MemModeX:
          res = 2;
          break;
        case MemMode8:
          out->reg_disp.r1 = bp;
          break;
        case MemMode16:
          out->reg_disp.r1 = bp;
          break;
        case RegMode:
          out->reg = regs_dec[rm][w];
          break;
      }
      break;
    case 0b111:
      out->reg_disp.r1 = bx;
      break;
    default:
      assert(0 && "unreachable _mod_rm_to_arg: " __FILE__);
  }

  switch (mod)
  {
    case MemModeX:
      if(rm == 0b110)
      {
        if(size < 2) goto bad;
        out->t = ArgMem;
        out->addr = (mem[1] << 8) + mem[0];
        res=2;
      }
      else if(rm < 0b100)
      {
        out->t = ArgMemRegRegDisp;
        res=0;
      }
      else
      {
        out->t = ArgReg;
        res=0;
      }
      break;
    case MemMode8:
      if(size < 1) goto bad;
      res=1;
      if(rm < 0b100)
      {
        out->t = ArgMemRegRegDisp;
        out->reg_reg_disp.disp.t = Disp8;
        out->reg_reg_disp.disp.disp8 = mem[0];
      }
      else
      {
        out->t = ArgMemRegDisp;
        out->reg_disp.disp.t = Disp8;
        out->reg_disp.disp.disp16 = mem[0];
      }
      break;
    case MemMode16:
      if(size < 2) goto bad;
      res=2;
      if(rm < 0b100)
      {
        out->t = ArgMemRegRegDisp;
        out->reg_reg_disp.disp.t = Disp16;
        out->reg_reg_disp.disp.disp16 = (mem[1] << 8) + mem[0];
      }
      else
      {
        out->t = ArgMemRegDisp;
        out->reg_disp.disp.t = Disp16;
        out->reg_disp.disp.disp16 = (mem[1] << 8) + mem[0];
      }
      break;
    case RegMode:
        out->t = ArgReg;
        res=0;
      break;
  }

  return res;

bad:
  res =-1;
  return res;
}

static inline s8 _imm_to_arg(const u8* mem, const u8 size, u8 w, Arg* out)
{
  s8 res=-1;

  if(w && size >= 2)
  {
    out->imm16 = (mem[1] << 8) +  mem[0];
    out->t = ArgImm16;
    res=2;
  }
  else if(!w && size >= 1)
  {
    out->imm8 = mem[0];
    out->t = ArgImm8;
    res=1;
  }

  return res;
}

static inline void _acc_to_arg(const u8 w, Arg* out)
{
  out->t = ArgReg;
  out->reg = w ? ax : al;
}

static inline s8 _addr_to_arg(const u8* mem, const u8 size, const u8 w, Arg* out)
{
  s8 res=0;
  out->t = ArgMem;

  if(size < (1<<w))  goto bad;
  res+= (1<<w);

  out->addr = w ? (mem[1] << 8) +  mem[0] : mem[0];
  
  return res;

bad:
  res =-1;
  return res;
}

static u32 _decode_mov_mem_to_acc(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=1;
  const u8 w = _get_w_bit(first_byte);
  s8 written;

  out->op = Opcode::mov;

  _acc_to_arg(w, &out->args[0]);

  written = _addr_to_arg(mem, size, w, &out->args[1]);
  if(written <0) goto bad;
  res+=written;


  return res;

bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

static u32 _decode_mov_acc_to_mem(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=1;
  const u8 w = _get_w_bit(first_byte);
  s8 written;

  out->op = Opcode::mov;

  written = _addr_to_arg(mem, size, w, &out->args[0]);
  if(written <0) goto bad;
  res+=written;

  _acc_to_arg(w, &out->args[1]);

  return res;

bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

static u32 _decode_mov_imm_to_reg_mem(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=0;

  const u8 w = first_byte & 0x1;
  ModField mod;
  u8 rm;
  u8 snd_byte;
  s8 written;

  out->op = Opcode::mov;

  if(size < 1) goto bad;

  snd_byte = mem[0];
  res++;

  mod = _get_mod_field(snd_byte);
  rm = _get_rm_field(snd_byte);

  written = _mod_rm_to_arg(mem + res, size - res, mod, rm, w, &out->args[0]);
  if(written < 0) goto bad;
  res+=written;

  written = _imm_to_arg(mem + res, size - res, w, &out->args[1]);
  if(written < 0) goto bad;
  res+=written;

  return res + 1;

bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

static u32 _decode_mov_imm_to_reg(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u32 res=1;
  const u8 w = (first_byte >> 3) & 0x1;
  const Register reg = (Register) (first_byte & 0x7);
  s8 written;
  Arg *arg1 = &out->args[0];
  Arg *arg2 = &out->args[1];

  out->op = Opcode::mov;
  arg1->t = ArgReg;
  arg1->reg = regs_dec[reg][w];

  written = _imm_to_arg(mem, size, w, arg2);
  if(written < 0) goto bad;

  res+=written;

  return res;

bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

static u32 _decode_mov_reg_mem_to_from_reg(
    const u8 first_byte, const u8* mem, const u32 size, Instruction* out)
{
  u32 res=1;
  u8 snd_byte=0;
  const u8 w = _get_w_bit(first_byte);
  const u8 d = _get_d_bit(first_byte);
  ModField mod;
  Register reg;
  u8 rm;
  Arg arg1;
  Arg arg2;
  s8 written=0;

  if(size <= 0) goto bad;

  snd_byte = mem[0];
  res++;

  mod = _get_mod_field(snd_byte);
  reg = _get_reg_field(snd_byte, w);
  rm = _get_rm_field(snd_byte);

  out->op = Opcode::mov;
  arg1.t = ArgReg;
  arg1.reg = reg;

  written = _mod_rm_to_arg(mem + 1, size -1, mod, rm, w, &arg2);
  if(written <0) goto bad;
  res+=written;

  if(d == 0)
  {
    out->args[0] = arg2;
    out->args[1] = arg1;
  }
  else
  {
    out->args[0] = arg1;
    out->args[1] = arg2;
  }

  return res;

bad:
  res=0;
  out->op = Opcode::INVALID;
  return res;
}

__attribute__((constructor))
static void _init_decoders_table(void) 
{
  for(u32 i=0; i<ArraySize(decoders); i++)
  {
    decoders[i] = _decode_invalid;
  }

  for(u8 i=0; i<4; i++)
  {
    decoders[0b10001000 + i] = _decode_mov_reg_mem_to_from_reg;
  }

  for(u8 i=0; i<= 0b1111; i++)
  {
    decoders[0b10110000 + i] = _decode_mov_imm_to_reg;
  }

  for(u8 i=0; i<= 0b1; i++)
  {
    decoders[0b11000110 + i] = _decode_mov_imm_to_reg_mem;
  }

  for(u8 i=0; i<= 0b1; i++)
  {
    decoders[0b10100000 + i] = _decode_mov_mem_to_acc;
  }

  for(u8 i=0; i<= 0b1; i++)
  {
    decoders[0b10100010 + i] = _decode_mov_acc_to_mem;
  }

}

static void _print_reg(const Register reg, FILE* out)
{
  switch (reg)
  {
#define X(r) case r: fprintf(out, #r); break;
  REGS
#undef X
    case INVALID_REG:
      assert(0 && "unreachable _print_reg: " __FILE__);
      break;
  }
}

static void _print_disp(const Displacement* disp, FILE *out)
{
  if(disp->t == Disp8 && disp->disp8)
  {
    fprintf(out, "%+d", disp->disp8);
  }
  else if(disp->t == Disp16 && disp->disp16)
  {
    fprintf(out, "%+d", disp->disp16);
  }
}

static void _print_arg(const Arg* const arg, FILE* out)
{
  switch (arg->t)
  {
    case ArgReg:
      _print_reg(arg->reg, out);
      break;
    case ArgMem:
      fprintf(out, "[%d]", arg->addr);
      break;
    case ArgImm8:
      fprintf(out, "byte %d", arg->imm8);
      break;
    case ArgImm16:
      fprintf(out, "word %d", arg->imm16);
      break;
    case ArgMemRegDisp:
      fprintf(out, "[");
      _print_reg(arg->reg_disp.r1, out);
      _print_disp(&arg->reg_disp.disp, out);
      fprintf(out, "]");

      break;
    case ArgMemRegRegDisp:
      fprintf(out, "[");
      _print_reg(arg->reg_reg_disp.r1, out);
      fprintf(out, "+");
      _print_reg(arg->reg_reg_disp.r2, out);
      _print_disp(&arg->reg_reg_disp.disp, out);
      fprintf(out, "]");

      break;
  }
}

void InstructionPrint(const Instruction& instr, FILE* out)
{
  assert(out);

  switch (instr.op)
  {
#define X(OP) case OP:  fprintf(out, #OP); break;
    OPS
#undef X
    case INVALID:
      fprintf(out, "INVALID OP");
      break;
  }

  fprintf(out, " ");
  _print_arg(&instr.args[0], out);
  fprintf(out, ", ");
  _print_arg(&instr.args[1], out);
  fprintf(out, "\n");
}

u32 InstructionDecode(const u8* mem, const u32 mem_size, Instruction* const out)
{
  u32 res = 0;
  u8 opcode;
  f_instruction_decoder decoder;

  if(mem_size < 1) return res;

  opcode = *mem;
  decoder = decoders[opcode];

  out->op = INVALID;
  res = decoder(opcode, mem + 1, mem_size -1, out);

  return res;
}
