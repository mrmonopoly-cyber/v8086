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
  return regs_dec[(byte >> 3) & 0x3][w];
}

static inline u8 _get_rm_field(const u8 byte)
{
  return (byte >> 0) & 0x7;
}

static inline u8 _mod_rm_to_arg(ModField mod, u8 rm, u8 w, Arg* out)
{
  u8 res = mod;
  out->reg_reg_disp.disp =0;

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
      if(mod == 0b00)
      {
        res = 2;
      }
      else if(mod == 0b11)
      {
        out->reg = regs_dec[rm][w];
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
        out->t = ArgMem;
      }
      else if(rm < 0b100)
      {
        out->t = ArgMemRegRegDisp;
      }
      else
      {
        out->t = ArgReg;
        res=0;
      }
      break;
    case MemMode8:
        out->t = ArgMemRegDisp;
      break;
    case MemMode16:
    case RegMode:
        out->t = ArgReg;
        res=0;
      break;
  }

  return res;
}

static u32 _decode_mov_reg_mem_to_from_reg(
    const u8 first_byte, const u8* mem, const u32 size, Instruction* out)
{
  u32 res=1;
  const u8 snd_byte = mem[0];
  const u8 w = _get_w_bit(first_byte);
  const u8 d = _get_d_bit(first_byte);
  const ModField mod = _get_mod_field(snd_byte);
  const Register reg = _get_reg_field(snd_byte, w);
  const u8 rm = _get_rm_field(snd_byte);
  Arg arg1;
  Arg arg2;
  u16 displacement=0;

  if(size >= 3)
  {
    out->op = Opcode::mov;
    arg1.t = ArgReg;
    arg1.reg = reg;

    res=2;
    switch(_mod_rm_to_arg(mod, rm, w, &arg2))
    {
      case 1:
        displacement  = mem[1];
        arg2.reg_disp.disp = displacement;
        res=3;
        break;
      case 2:
        displacement  = (mem[2] << 8) + mem[1];
        arg2.reg_reg_disp.disp = displacement;
        res=4;
        break;
    }

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

  }


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

}

static void _print_reg(const Register reg, FILE* out)
{
  switch (reg)
  {
#define X(r) case r: fprintf(out, #r); break;
  REGS
#undef X
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
      TODO();
      break;
    case ArgImm8:
      TODO();
      break;
    case ArgImm16:
      TODO();
      break;
    case ArgMemRegDisp:
      TODO();
      break;
    case ArgMemRegRegDisp:
      fprintf(out, "[");
      _print_reg(arg->reg_reg_disp.r1, out);
      fprintf(out, "+");
      _print_reg(arg->reg_reg_disp.r2, out);
      fprintf(out, "+%d]", arg->reg_reg_disp.disp);
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
