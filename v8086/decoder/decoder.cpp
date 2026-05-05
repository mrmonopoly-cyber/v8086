#include "decoder.h"

#include <assert.h>
#include <stdio.h>

#include <v8086_definitions.h>

enum ModField{
  MemModeX  =  0b00,
  MemMode8  =  0b01,
  MemMode16 =  0b10,
  RegMode   =  0b11,
};

enum class RepArgOp
{
  movsb = 0b10100100, movsw = 0b10100101,
  cmpsb = 0b10100110, cmpsw = 0b10100111,
  scasb = 0b10101110, scasw = 0b10101111,
  lodsb = 0b10101100, lodsw = 0b10101101,
  stosb = 0b10101010, stosw = 0b10101011,
};

typedef u32 (*f_instruction_decoder)(
    const u8 first_byte,
    const u8* const mem,
    const u32 size,
    Instruction* out);

static Register regs_dec[][2] =
{
  {dec_al, dec_ax},
  {dec_cl, dec_cx},
  {dec_dl, dec_dx},
  {dec_bl, dec_bx},
  {dec_ah, dec_sp},
  {dec_ch, dec_bp},
  {dec_dh, dec_si},
  {dec_bh, dec_di},
};

static f_instruction_decoder decoders[256];

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

static inline u8 _get_s_bit(const u8 byte)
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

static inline s8 _mod_rm_to_arg(const u8* mem, u32 size, ModField mod, u8 rm, u8 w, Arg* out, u8 far=0)
{
  s8 res = mod;

  switch (rm)
  {
    case 0b000:
      out->reg_reg_disp.r1 = dec_bx;
      out->reg_reg_disp.r2 = dec_si;
      break;
    case 0b001:
      out->reg_reg_disp.r1 = dec_bx;
      out->reg_reg_disp.r2 = dec_di;
      break;
    case 0b010:
      out->reg_reg_disp.r1 = dec_bp;
      out->reg_reg_disp.r2 = dec_si;
      break;
    case 0b011:
      out->reg_reg_disp.r1 = dec_bp;
      out->reg_reg_disp.r2 = dec_di;
      break;
    case 0b100:
      out->reg_disp.r1 = dec_si;
      break;
    case 0b101:
      out->reg_disp.r1 = dec_di;
      break;
    case 0b110:
      switch (mod)
      {
        case MemModeX:
          res = 2;
          break;
        case MemMode8:
          out->reg_disp.r1 = dec_bp;
          break;
        case MemMode16:
          out->reg_disp.r1 = dec_bp;
          break;
        case RegMode:
          out->reg = regs_dec[rm][w];
          break;
      }
      break;
    case 0b111:
      out->reg_disp.r1 = dec_bx;
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
        out->addr.addr = (mem[1] << 8) + mem[0];
        out->addr.word = w;
        res=2;
      }
      else if(rm < 0b100)
      {
        out->t = ArgMemRegRegDisp;
        out->reg_reg_disp.disp.word = w;
        out->reg_reg_disp.disp.far = far;
        res=0;
      }
      else
      {
        out->t = ArgMemRegDisp;
        out->reg_disp.disp.word = w;
        out->reg_disp.disp.far = far;
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
        out->reg_reg_disp.disp.word = w;
        out->reg_reg_disp.disp.far = far;
      }
      else
      {
        out->t = ArgMemRegDisp;
        out->reg_disp.disp.t = Disp8;
        out->reg_disp.disp.disp16 = mem[0];
        out->reg_disp.disp.word = w;
        out->reg_disp.disp.far = far;
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
        out->reg_reg_disp.disp.word = w;
        out->reg_reg_disp.disp.far = far;
      }
      else
      {
        out->t = ArgMemRegDisp;
        out->reg_disp.disp.t = Disp16;
        out->reg_disp.disp.disp16 = (mem[1] << 8) + mem[0];
        out->reg_disp.disp.word = w;
        out->reg_disp.disp.far = far;
      }
      break;
    case RegMode:
        out->t = ArgReg;
        out->reg = regs_dec[rm][w];
        res=0;
      break;
  }

  return res;

bad:
  res =-1;
  return res;
}

static s8 _reg_mem_to_either(const u8 *mem, u32 size, const u8 w, const u8 d, Instruction* out)
{
  s8 res=0;
  u8 snd_byte;
  ModField mod;
  u8 rm;
  s8 written;
  Arg arg1;
  Arg arg2;
  Register reg;

  if(size <= 0) goto bad;

  snd_byte = mem[0];
  res++;

  mod = _get_mod_field(snd_byte);
  reg = _get_reg_field(snd_byte, w);
  rm = _get_rm_field(snd_byte);

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
  res=-1;
  return res;
}

static inline s8 _imm_to_arg(const u8* mem, const u32 size, u8 w, Arg* out)
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
  out->reg = w ? dec_ax : dec_al;
}

static inline s8 _addr_to_arg(const u8* mem, const u32 size, const u8 w, Arg* out)
{
  s8 res=0;
  out->t = ArgMem;

  if(size < (1u<<w))  goto bad;
  res+= (1<<w);

  out->addr.addr = w ? (mem[1] << 8) +  mem[0] : mem[0];
  out->addr.word = w;
  
  return res;

bad:
  res =-1;
  return res;
}

static s8 _imm_to_reg_mem(
    const u8* mem,
    const u32 size,
    const u8 s,
    const u8 w,
    Instruction* out,
    Opcode op = Opcode::INVALID)
{
  s8 res=0;
  u8 snd_byte;
  ModField mod;
  u8 rm;
  s8 written;

  if(size < 1) goto bad;

  snd_byte = mem[0];
  res++;

  if(op != Opcode::INVALID)
  {
    out->op = op;
  }
  else
  {
    switch ((snd_byte >> 3) & 0x7)
    {
      case 0b000:
        out->op = Opcode::add;
        break;
      case 0b001:
        out->op = Opcode::OpOr;
        break;
      case 0b110: //HACK: reverse engineering of nasm. I do not know why it works at the moment
        out->op = Opcode::OpXor;
        break;
      case 0b100:
        out->op = Opcode::OpAnd;
        break;
      case 0b010:
        out->op = Opcode::adc;
        break;
      case 0b101:
        out->op = Opcode::sub;
        break;
      case 0b011:
        out->op = Opcode::sbb;
        break;
      case 0b111:
        out->op = Opcode::cmp;
        break;

      default:
        fprintf(stderr, "op: %d\n", (snd_byte >> 3) & 0x7);
        assert(0 && "unreachable _imm_to_reg_mem " __FILE__);
    
    }
  }

  mod = _get_mod_field(snd_byte);
  rm = _get_rm_field(snd_byte);

  written = _mod_rm_to_arg(mem + res, size - res, mod, rm, w, &out->args[0]);
  if(written < 0) goto bad;
  res+=written;

  if(out->op == Opcode::OpAnd || out->op == Opcode::OpOr)
  {
    written = _imm_to_arg(mem + res, size - res, w, &out->args[1]);
  }
  else
  {
    written = _imm_to_arg(mem + res, size - res, (!s) * w, &out->args[1]);
  }

  if(written < 0) goto bad;
  res+=written;

  return res;

bad:
  res =-1;
  return res;
}

static s8 _imm_to_acc(const u8* mem, const u32 size, const u8 w, Instruction* out)
{
  s8 res=0;
  s8 written;

  _acc_to_arg(w, &out->args[0]);

  written = _imm_to_arg(mem, size, w, &out->args[1]);
  if(written <0) goto bad;
  res+=written;

  return res;

bad:
  res =-1;
  return res;
}

static u8 _cycles_ea(Arg* arg)
{
  u8 res=0;

  switch (arg->t)
  {
    case ArgMem:
      res = 6;
      break;
    case ArgMemRegDisp:
      res = 5;
      if(arg->reg_disp.disp.disp16)
      {
        res+=4;
      }
      break;
    case ArgMemRegRegDisp:
      if
        (
          (arg->reg_reg_disp.r1 == Register::dec_bp && arg->reg_reg_disp.r2 == Register::dec_di) ||
          (arg->reg_reg_disp.r1 == Register::dec_bx && arg->reg_reg_disp.r2 == Register::dec_si)
        )
      {
        res = 7;
      }
      else if
        (
          (arg->reg_reg_disp.r1 == Register::dec_bp && arg->reg_reg_disp.r2 == Register::dec_di) ||
          (arg->reg_reg_disp.r1 == Register::dec_bx && arg->reg_reg_disp.r2 == Register::dec_si)
        )
      {
        res = 8;
      }
      if(arg->reg_reg_disp.disp.disp16)
      {
        res+=4;
      }
      break;
    default:
      assert(0 && "unreachable");
  }

  return res;
}

#define ArgReg(t) ((t) == ArgType::ArgReg)

#define ArgMem(t) \
  ((t) == ArgType::ArgMem || \
   (t) == ArgType::ArgMemRegDisp || \
   (t) == ArgType::ArgMemRegRegDisp)

#define ArgImm(t) \
  ((t) == ArgType::ArgImm8 || \
   (t) == ArgType::ArgImm16 || \
   (t) == ArgType::ArgUImm8 || \
   (t) == ArgType::ArgUImm16)

static void _cycles_mov(Instruction* out)
{
  const ArgType t1 = out->args[0].t;
  const ArgType t2 = out->args[1].t;

  if(ArgReg(t1) && ArgReg(t2))
  {
    out->cycles = 2;
  }
  else if (ArgReg(t1) && ArgMem(t2))
  {
    out->cycles = 8 + _cycles_ea(&out->args[1]);
  }
  else if (ArgMem(t1) && ArgReg(t2))
  {
    out->cycles = 9 + _cycles_ea(&out->args[0]);
  }
  else if (ArgReg(t1) && ArgImm(t2))
  {
    out->cycles = 4;
  }
  else
  {
    fprintf(stderr, "unamanged case for cycles mov (%d, %d)\n", t1, t2); 
    assert(0 && "unreachable");
  }

}

static void _cycles_add(Instruction* out)
{
  const ArgType t1 = out->args[0].t;
  const ArgType t2 = out->args[1].t;

  if(ArgReg(t1) && ArgReg(t2))
  {
    out->cycles = 3;
  }
  else if (ArgReg(t1) && ArgMem(t2))
  {
    out->cycles = 9 + _cycles_ea(&out->args[1]);
  }
  else if (ArgMem(t1) && ArgReg(t2))
  {
    out->cycles = 16 + _cycles_ea(&out->args[0]);
  }
  else if (ArgMem(t1) && ArgImm(t2))
  {
    out->cycles = 17;
  }
  else if (ArgReg(t1) && ArgImm(t2))
  {
    out->cycles = 4;
  }
  else
  {
    fprintf(stderr, "unamanged case for cycles add (%d, %d)\n", t1, t2); 
    assert(0 && "unreachable");
  }

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
  if(written < 0) goto bad;
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
  s8 written;

  written = _imm_to_reg_mem(mem, size, 0, w, out, Opcode::mov);
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

static u32 _decode_mov_reg_mem_to_seg_reg(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u32 res=1;
  ModField mod;
  u8 rm;
  u8 snd_byte;
  s8 written;

  UNUSED(first_byte);

  if(size < 1)goto bad;
  snd_byte = mem[0];
  res++;

  out->op = Opcode::mov;

  mod = _get_mod_field(snd_byte);
  rm = _get_rm_field(snd_byte);

  written = _mod_rm_to_arg(mem + 1, size -1, mod, rm, 1, &out->args[0]);
  if(written < 0) goto bad;
  res+=written;

  out->args[1].t = ArgSegment;
  out->args[1].seg = (Segment) ((snd_byte >> 3) & 0x3);


  return res;

bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

static u32 _decode_mov_seg_reg_to_reg_mem(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u32 res=1;
  ModField mod;
  u8 rm;
  u8 snd_byte;
  s8 written;

  UNUSED(first_byte);

  if(size < 1)goto bad;
  snd_byte = mem[0];
  res++;

  out->op = Opcode::mov;

  out->args[0].t = ArgSegment;
  out->args[0].seg = (Segment) ((snd_byte >> 3) & 0x3);

  mod = _get_mod_field(snd_byte);
  rm = _get_rm_field(snd_byte);

  written = _mod_rm_to_arg(mem + 1, size -1, mod, rm, 1, &out->args[1]);
  if(written < 0) goto bad;
  res+=written;

  return res;

bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

#define DECODE_REG_MEM_TO_EITHER(opcode)                                        \
static u32 _decode_##opcode##_reg_mem_to_from_reg(                              \
    const u8 first_byte, const u8* mem, const u32 size, Instruction* out)       \
{                                                                               \
  u32 res=1;                                                                    \
  const u8 w = _get_w_bit(first_byte);                                          \
  const u8 d = _get_d_bit(first_byte);                                          \
  s8 written=0;                                                                 \
  out->op = Opcode::opcode;                                                     \
  written = _reg_mem_to_either(mem, size, w, d, out);                           \
  if(written < 0) goto bad;                                                     \
  res+=written;                                                                 \
  return res;                                                                   \
bad:                                                                            \
  res=0;                                                                        \
  out->op = Opcode::INVALID;                                                    \
  return res;                                                                   \
}

DECODE_REG_MEM_TO_EITHER(mov);
DECODE_REG_MEM_TO_EITHER(add);
DECODE_REG_MEM_TO_EITHER(adc);
DECODE_REG_MEM_TO_EITHER(sub);
DECODE_REG_MEM_TO_EITHER(sbb);
DECODE_REG_MEM_TO_EITHER(cmp);
DECODE_REG_MEM_TO_EITHER(OpAnd);
DECODE_REG_MEM_TO_EITHER(test);
DECODE_REG_MEM_TO_EITHER(OpOr);
DECODE_REG_MEM_TO_EITHER(OpXor);


static u32 _decode_arithm_imm_to_reg_mem(
    const u8 first_byte, const u8* mem, const u32 size, Instruction* out)
{
  u32 res=1;
  const u8 w = _get_w_bit(first_byte);
  const u8 s = _get_s_bit(first_byte);
  s8 written=0;

  written = _imm_to_reg_mem(mem, size, s, w, out);
  if(written < 0) goto bad;
  res+=written;

  return res;

bad:
  res=0;
  out->op = Opcode::INVALID;
  return res;
}

#define DECODE_ARITH_IMM_TO_ACC(opcode)                                         \
static u32 _decode_##opcode##_imm_to_acc(                                       \
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)             \
{                                                                               \
  u8 res=1;                                                                     \
  const u8 w = _get_w_bit(first_byte);                                          \
  s8 written;                                                                   \
  out->op = Opcode::opcode;                                                     \
  written = _imm_to_acc(mem, size, w, out);                                     \
  if(written < 0) goto bad;                                                     \
  res+=written;                                                                 \
  return res;                                                                   \
bad:                                                                            \
  res =0;                                                                       \
  out->op = Opcode::INVALID;                                                    \
  return res;                                                                   \
}

DECODE_ARITH_IMM_TO_ACC(add);
DECODE_ARITH_IMM_TO_ACC(adc);
DECODE_ARITH_IMM_TO_ACC(sub);
DECODE_ARITH_IMM_TO_ACC(sbb);
DECODE_ARITH_IMM_TO_ACC(cmp);
DECODE_ARITH_IMM_TO_ACC(OpAnd);
DECODE_ARITH_IMM_TO_ACC(OpOr);
DECODE_ARITH_IMM_TO_ACC(OpXor);
DECODE_ARITH_IMM_TO_ACC(test); //INFO: the manual is wrong data has also the w option

#define TEMPLATE_DECODER_JMP(name, opcode)                                                        \
static u32 _decode_##name (                                                                       \
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)                               \
{                                                                                                 \
  u8 res=1;                                                                                       \
  s8 written;                                                                                     \
  UNUSED(first_byte);                                                                             \
  out->op = Opcode::opcode;                                                                               \
  written = _imm_to_arg(mem, size, 0, &out->args[0]);                                             \
  if(written<0) goto bad;                                                                         \
  res+=written;                                                                                   \
  return res;                                                                                     \
bad:                                                                                              \
  res =0;                                                                                         \
  out->op = Opcode::INVALID;                                                                      \
  return res;                                                                                     \
}

TEMPLATE_DECODER_JMP(je_jz, jz)
TEMPLATE_DECODER_JMP(jl_jnge, jz)
TEMPLATE_DECODER_JMP(jle_jng, jle)
TEMPLATE_DECODER_JMP(jb_jnae, jb)
TEMPLATE_DECODER_JMP(jbe_jna, jbe)
TEMPLATE_DECODER_JMP(jp_jpe, jp)
TEMPLATE_DECODER_JMP(jo, jo)
TEMPLATE_DECODER_JMP(js, js)
TEMPLATE_DECODER_JMP(jne_jnz, jnz)
TEMPLATE_DECODER_JMP(jnl_jge, jnl)
TEMPLATE_DECODER_JMP(jnle_jg, jnle)
TEMPLATE_DECODER_JMP(jnb_jae, jnb)
TEMPLATE_DECODER_JMP(jnbe_ja, jnbe)
TEMPLATE_DECODER_JMP(jnp_jpo, jnp)
TEMPLATE_DECODER_JMP(jno, jno)
TEMPLATE_DECODER_JMP(jns, jns)
TEMPLATE_DECODER_JMP(loop, loop)
TEMPLATE_DECODER_JMP(loopz_loope, loopz)
TEMPLATE_DECODER_JMP(loopnz_loopne, loopnz)
TEMPLATE_DECODER_JMP(jcxz, jcxz)


static u32 _decode_inc_dec_reg_mem(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=1;
  const u8 w = _get_w_bit(first_byte);
  u8 snd_byte;
  s8 written;
  ModField mod;
  u8 rm;


  if(size < 1) goto bad;
  snd_byte = mem[0];
  res++;

  switch ((snd_byte >> 3) & 0x7)
  {
    case 0b001:
      out->op = Opcode::dec;
      break;
    case 0b000:
      out->op = Opcode::inc;
      break;
    default:
      assert(0 && "unreachable _decode_inc_dec_reg_mem");
  }

  mod = _get_mod_field(snd_byte);
  rm = _get_rm_field(snd_byte);

  written = _mod_rm_to_arg(mem +1, size -1, mod, rm, w, &out->args[0]);
  if(written < 0) goto bad;
  res += written;

  return res;

bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

static u32 _decode_neg_mul_div_imul_idiv_reg_mem(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=1;
  const u8 w = _get_w_bit(first_byte);
  u8 snd_byte;
  s8 written;
  ModField mod;
  u8 rm;


  if(size < 1) goto bad;
  snd_byte = mem[0];
  res++;

  switch ((snd_byte >> 3) & 0x7)
  {
    case 0b000:
      out->op = Opcode::test;
      break;
    case 0b100:
      out->op = Opcode::mul;
      break;
    case 0b101:
      out->op = Opcode::imul;
      break;

    case 0b110:
      out->op = Opcode::div;
      break;
    case 0b111:
      out->op = Opcode::idiv;
      break;

    case 0b010:
      out->op = Opcode::OpNot;
      break;
    case 0b011:
      out->op = Opcode::neg;
      break;

    default:
      fprintf(stderr, "opcode %d\n", (snd_byte >> 3) & 0x7);
      assert(0 && "unreachable _decode_neg_mul_div_imul_idiv_reg_mem");
      break;
  }

  mod = _get_mod_field(snd_byte);
  rm = _get_rm_field(snd_byte);

  if(out->op == Opcode::test)
  {
    res=1;
    written = _imm_to_reg_mem(mem, size, 1, w, out, Opcode::test);
    if(written < 0) goto bad;
    res += written;
  }
  else
  {
    written = _mod_rm_to_arg(mem +1, size -1, mod, rm, w, &out->args[0]);
    if(written < 0) goto bad;
    res += written;
  }


  return res;

bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

static u32 _decode_indirect_intersegment(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=0;
  u8 snd_byte;
  s8 written;
  ModField mod;
  u8 specialization;
  u8 rm;
  u8 w=0;
  u8 far=0;

  UNUSED(first_byte);

  if(size < 1) goto bad;

  snd_byte = mem[0];
  res++;

  mod = (ModField) ((snd_byte >> 6) & 0x7);
  specialization = (snd_byte >> 3) & 0x7;
  rm = snd_byte & 0x7;

  switch (specialization)
  {
    case 0b000:
      return _decode_inc_dec_reg_mem(first_byte, mem, size, out);
      break;
    case 0b001:
      return _decode_inc_dec_reg_mem(first_byte, mem, size, out);
      break;
    case 0b110:
      out->op = Opcode::push;
      break;
    case 0b010:
      out->op = Opcode::call; //INFO: within segment
      w=1;
      break;
    case 0b011:
      out->op = Opcode::call; //INFO: inter segment
      w=1;
      far=1;
      break;
    case 0b100:
      out->op = Opcode::jmp;  //INFO: within segment
      w=1;
      break;
    case 0b101:
      out->op = Opcode::jmp; //INFO: inter segment
      w=1;
      far=1;
      break;
    default:
      fprintf(stderr, "op: %d\n", specialization);
      assert(0 && "unreachable _decode_indirect_intersegment" __FILE__);

  }

  written = _mod_rm_to_arg(mem + res, size - res, mod, rm, w, &out->args[0], far);
  if(written < 0) goto bad;
  res+=written;


  return res + 1;

bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

static u32 _decode_push_register(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=0;
  Register reg = regs_dec[first_byte & 0x7][1];

  UNUSED(mem);
  UNUSED(size);

  out->op = Opcode::push;
  out->args[0].t = ArgReg;
  out->args[0].reg = reg;

  return res + 1;
}

static u32 _decode_pop_register(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=0;
  Register reg = regs_dec[first_byte & 0x7][1];

  UNUSED(mem);
  UNUSED(size);

  out->op = Opcode::pop;
  out->args[0].t = ArgReg;
  out->args[0].reg = reg;

  return res + 1;
}

static u32 _decode_push_segment_register(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=0;
  Segment seg =  (Segment) ((first_byte >> 3) & 0x3);

  UNUSED(mem);
  UNUSED(size);

  out->op = Opcode::push;
  out->args[0].t = ArgSegment;
  out->args[0].seg = seg;

  return res + 1;
}

static u32 _decode_pop_segment_register(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=0;
  Segment seg =  (Segment) ((first_byte >> 3) & 0x3);

  UNUSED(mem);
  UNUSED(size);

  out->op = Opcode::pop;
  out->args[0].t = ArgSegment;
  out->args[0].seg = seg;

  return res + 1;
}

static u32 _decode_pop_reg_mem(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=0;
  u8 snd_byte;
  ModField mod;
  u8 rm;
  s8 written;
  UNUSED(first_byte);
  if(size < 1) goto bad;
  snd_byte = mem[0];
  res++;
  out->op = Opcode::pop;
  mod = _get_mod_field(snd_byte);
  rm = _get_rm_field(snd_byte);
  written = _mod_rm_to_arg(mem + res, size - res, mod, rm, 0, &out->args[0]);
  if(written<0) goto bad;
  res+=written;
  return res + 1;
bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

static u32 _decode_xchg_reg_mem_with_reg(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=1;
  s8 written;
  const u8 w = _get_w_bit(first_byte);

  out->op = Opcode::xchg;

  written = _reg_mem_to_either(mem, size, w, 1, out);
  if(written<0) goto bad;
  res += written;

  return res;

bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

static u32 _decode_xchg_reg_with_acc(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=1;
  const Register reg = regs_dec[first_byte & 0x7][1];

  UNUSED(mem);
  UNUSED(size);

  out->op = Opcode::xchg;

  out->args[0].t = ArgReg;
  out->args[0].reg = dec_ax;

  out->args[1].t = ArgReg;
  out->args[1].reg = reg;

  return res;
}

static u32 _decode_in_fixed_port(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=1;
  const u8 w = _get_w_bit(first_byte);
  u8 data;

  out->op = Opcode::in;

  if(size < 1) goto bad;
  data = mem[0];
  res++;

  out->args[0].t = ArgReg;
  out->args[0].reg = w ? dec_ax : dec_al;

  out->args[1].t = ArgUImm8;
  out->args[1].uimm8 = data;

  return res;

bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

static u32 _decode_in_variable_port(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=1;
  const u8 w = _get_w_bit(first_byte);

  UNUSED(mem);
  UNUSED(size);

  out->op = Opcode::in;

  out->args[0].t = ArgReg;
  out->args[0].reg = w ? dec_ax : dec_al;

  out->args[1].t = ArgReg;
  out->args[1].reg = dec_dx;

  return res;
}

static u32 _decode_out_fixed_port(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=1;
  const u8 w = _get_w_bit(first_byte);
  u8 data;

  out->op = Opcode::out;

  if(size < 1) goto bad;
  data = mem[0];
  res++;

  out->args[0].t = ArgUImm8;
  out->args[0].uimm8 = data;

  out->args[1].t = ArgReg;
  out->args[1].reg = w ? dec_ax : dec_al;


  return res;

bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

static u32 _decode_out_variable_port(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=1;
  const u8 w = _get_w_bit(first_byte);

  UNUSED(mem);
  UNUSED(size);

  out->op = Opcode::out;

  out->args[0].t = ArgReg;
  out->args[0].reg = dec_dx;

  out->args[1].t = ArgReg;
  out->args[1].reg = w ? dec_ax : dec_al;


  return res;
}

#define TEMPLATE_DECODE_ONLY_FIRST_BYTE(opcode)                                    \
static u32 _decode_##opcode(                                              \
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)       \
{                                                                         \
  u8 res=1;                                                               \
  UNUSED(mem);                                                            \
  UNUSED(size);                                                           \
  UNUSED(first_byte);                                                     \
  out->op = Opcode::opcode;                                               \
  return res;                                                             \
}

TEMPLATE_DECODE_ONLY_FIRST_BYTE(xlat)
TEMPLATE_DECODE_ONLY_FIRST_BYTE(lahf)
TEMPLATE_DECODE_ONLY_FIRST_BYTE(sahf)
TEMPLATE_DECODE_ONLY_FIRST_BYTE(pushf)
TEMPLATE_DECODE_ONLY_FIRST_BYTE(popf)
TEMPLATE_DECODE_ONLY_FIRST_BYTE(aaa)
TEMPLATE_DECODE_ONLY_FIRST_BYTE(daa)
TEMPLATE_DECODE_ONLY_FIRST_BYTE(aas)
TEMPLATE_DECODE_ONLY_FIRST_BYTE(das)
TEMPLATE_DECODE_ONLY_FIRST_BYTE(cbw);
TEMPLATE_DECODE_ONLY_FIRST_BYTE(cwd);
TEMPLATE_DECODE_ONLY_FIRST_BYTE(ret);
TEMPLATE_DECODE_ONLY_FIRST_BYTE(retf);
TEMPLATE_DECODE_ONLY_FIRST_BYTE(int3);
TEMPLATE_DECODE_ONLY_FIRST_BYTE(into);
TEMPLATE_DECODE_ONLY_FIRST_BYTE(iret);
TEMPLATE_DECODE_ONLY_FIRST_BYTE(clc);
TEMPLATE_DECODE_ONLY_FIRST_BYTE(cmc);
TEMPLATE_DECODE_ONLY_FIRST_BYTE(stc);
TEMPLATE_DECODE_ONLY_FIRST_BYTE(cld);
TEMPLATE_DECODE_ONLY_FIRST_BYTE(std);
TEMPLATE_DECODE_ONLY_FIRST_BYTE(cli);
TEMPLATE_DECODE_ONLY_FIRST_BYTE(sti);
TEMPLATE_DECODE_ONLY_FIRST_BYTE(hlt);
TEMPLATE_DECODE_ONLY_FIRST_BYTE(wait);

static u32 _decode_int_type_specified(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=1;

  UNUSED(first_byte);

  if( size < 1 ) goto bad;
  res++;

  out->op = Opcode::OpInt;

  out->args[0].t = ArgImm8;
  out->args[0].imm8 =  mem[0];

  return res;

bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

static u32 _decode_ret_XX_seg_add_imm_to_sp(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=1;

  UNUSED(first_byte);

  if( size < 2 ) goto bad;
  res+=2;

  out->op = Opcode::ret;

  out->args[0].t = ArgImm16;
  out->args[0].imm16 =  (mem[1] << 8) + mem[0];

  return res;

bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

static u32 _decode_retf_XX_seg_add_imm_to_sp(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=1;

  UNUSED(first_byte);

  if( size < 2 ) goto bad;
  res+=2;

  out->op = Opcode::retf;

  out->args[0].t = ArgImm16;
  out->args[0].imm16 =  (mem[1] << 8) + mem[0];

  return res;

bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

static u32 _decode_aam(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=1;
  u8 snd_byte;

  UNUSED(first_byte);

  if( size < 1 ) goto bad;
  snd_byte = mem[0];
  res++;

  switch (snd_byte)
  {
    case 0b00001010:
      out->op = Opcode::aam;
      break;
    default:
      fprintf(stderr, "opcode: %d\n", snd_byte);
      assert(0 && "unreachable _decode_aam");
  }

  return res;

bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

static u32 _decode_aad(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=1;
  u8 snd_byte;

  UNUSED(first_byte);

  if( size < 1 ) goto bad;
  snd_byte = mem[0];
  res++;

  switch (snd_byte)
  {
    case 0b00001010:
      out->op = Opcode::aad;
      break;
    default:
      fprintf(stderr, "opcode: %d\n", snd_byte);
      assert(0 && "unreachable _decode_aam");
  }

  return res;

bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

static u32 _decode_logic_shift_rotate(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=1;
  u8 snd_byte;
  const u8 w = _get_w_bit(first_byte);
  const u8 v = _get_d_bit(first_byte);
  ModField mod;
  u8 rm;
  s8 written;

  if( size < 1 ) goto bad;
  snd_byte = mem[0];
  res++;

  mod = _get_mod_field(snd_byte);
  rm = _get_rm_field(snd_byte);

      switch ((snd_byte >> 3) & 0x7)
    {
      case 0b000:
        out->op = Opcode::rol;
        break;
      case 0b001:
        out->op = Opcode::ror;
        break;
      case 0b010:
        out->op = Opcode::rcl;
        break;
      case 0b011:
        out->op = Opcode::rcr;
        break;
      case 0b100:
        out->op = Opcode::shl;
        break;
      case 0b101:
        out->op = Opcode::shr;
        break;
      case 0b111:
        out->op = Opcode::sar;
        break;
      default:
        fprintf(stderr, "op: %d\n", (snd_byte >> 3) & 0x7);
        assert(0 && "unreachable _decode_logic_shift_rotate");
    
    }


  mod = _get_mod_field(snd_byte);
  rm = _get_rm_field(snd_byte);

  written = _mod_rm_to_arg(mem + 1 , size - 1, mod, rm, w, &out->args[0]);
  if(written < 0) goto bad;
  res+=written;

  if(!v)
  {
    out->args[1].t = ArgImm8;
    out->args[1].imm8 = 1;
  }
  else
  {
    out->args[1].t = ArgReg;
    out->args[1].imm8 = dec_cl;
  }

  return res;

bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

#define TEMPLATE_DIR_INTER_SEG(opcode)                                  \
static u32 _decode_##opcode##_direct_intersegment(                      \
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)     \
{                                                                       \
  u8 res=1;                                                             \
  UNUSED(first_byte);                                                   \
  if(size < 4) goto bad;                                                \
  out->op = Opcode::opcode;                                             \
  out->args[0].t = ArgDirInterSeg;                                      \
  out->args[0].dir_inter_seg.addr[0] = (mem[3] << 8) + mem[2];          \
  out->args[0].dir_inter_seg.addr[1] = (mem[1] << 8) + mem[0];          \
  res+=4;                                                               \
  return res;                                                           \
bad:                                                                    \
  res =0;                                                               \
  out->op = Opcode::INVALID;                                            \
  return res;                                                           \
}

#define TEMPLATE_DIR_INTER_SEG_SHORT(opcode)                            \
static u32 _decode_##opcode##_direct_intersegment_short(                \
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)     \
{                                                                       \
  u8 res=1;                                                             \
  UNUSED(first_byte);                                                   \
  if(size < 1) goto bad;                                                \
  out->op = Opcode::opcode;                                             \
  out->args[0].t = ArgIpInc8;                                           \
  out->args[0].ip_inc_8 = mem[0];                                       \
  res++;                                                                \
  return res;                                                           \
bad:                                                                    \
  res =0;                                                               \
  out->op = Opcode::INVALID;                                            \
  return res;                                                           \
}

TEMPLATE_DIR_INTER_SEG(call);
TEMPLATE_DIR_INTER_SEG(jmp);
TEMPLATE_DIR_INTER_SEG_SHORT(jmp);

#define TEMPLATE_DIR_WITHIN_SEG(opcode)                                 \
static u32 _decode_##opcode##_direct_withinseg(                         \
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)     \
{                                                                       \
  u8 res=1;                                                             \
  UNUSED(first_byte);                                                   \
  if(size < 2) goto bad;                                                \
  out->op = Opcode::opcode;                                             \
  out->args[0].t = ArgIpInc16;                                          \
  out->args[0].ip_inc_16 = (mem[1] << 8) + mem[0];                      \
  res+=2;                                                               \
  return res;                                                           \
bad:                                                                    \
  res =0;                                                               \
  out->op = Opcode::INVALID;                                            \
  return res;                                                           \
}

TEMPLATE_DIR_WITHIN_SEG(jmp);
TEMPLATE_DIR_WITHIN_SEG(call);

static u32 _decode_repeate(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=1;
  u8 snd_byte;
  RepArgOp op;

  UNUSED(first_byte);

  out->prefix = Prefix::rep;

  if( size < 1 ) goto bad;
  snd_byte = mem[0];
  res++;

  op = (RepArgOp) snd_byte;

  switch (op)
  {
    case RepArgOp::movsb: out->op = Opcode::movsb; break;
    case RepArgOp::movsw: out->op = Opcode::movsw; break;
    case RepArgOp::cmpsb: out->op = Opcode::cmpsb; break;
    case RepArgOp::cmpsw: out->op = Opcode::cmpsw; break;
    case RepArgOp::scasb: out->op = Opcode::scasb; break;
    case RepArgOp::scasw: out->op = Opcode::scasw; break;
    case RepArgOp::lodsb: out->op = Opcode::lodsb; break;
    case RepArgOp::lodsw: out->op = Opcode::lodsw; break;
    case RepArgOp::stosb: out->op = Opcode::stosb; break;
    case RepArgOp::stosw: out->op = Opcode::stosw; break;
  }

  return res;

bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

static u32 _decode_lock(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=0;
  u8 snd_byte;
  s8 written;

  UNUSED(first_byte);

  out->prefix = Prefix::lock;

  if( size < 1 ) goto bad;
  snd_byte = mem[0];
  res++;

  written = decoders[snd_byte](snd_byte, mem +1, size -1, out);
  if(written <0) goto bad;
  res+=written;

  return res;

bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

static u32 _decode_seg_prefix(
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)
{
  u8 res=1;
  s8 written;
  u8 snd_byte;
  const Segment seg = (Segment)((first_byte >> 3) & 0x3);

  out->seg = seg;

  if(size < 1) goto bad;
  snd_byte = mem[0];

  written = decoders[snd_byte](snd_byte, mem + 1, size -1, out);
  if(written <= 0) goto bad;
  res+=written;

  return res;
bad:
  res =0;
  out->op = Opcode::INVALID;
  return res;
}

#define TEMPLATE_LXX_TO_REG(opcode)                                     \
static u32 _decode_##opcode##_to_reg(                                   \
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)     \
{                                                                       \
  u8 res=1;                                                             \
  s8 written;                                                           \
  UNUSED(mem);                                                          \
  UNUSED(size);                                                         \
  UNUSED(first_byte);                                                   \
  out->op = Opcode::opcode;                                             \
  written = _reg_mem_to_either(mem, size, 1, 1, out);                   \
  if(written < 0) goto bad;                                             \
  res+=written;                                                         \
  return res;                                                           \
bad:                                                                    \
  res =0;                                                               \
  out->op = Opcode::INVALID;                                            \
  return res;                                                           \
}

TEMPLATE_LXX_TO_REG(lea)
TEMPLATE_LXX_TO_REG(lds)
TEMPLATE_LXX_TO_REG(les)



#define TEMPLATE_ONE_BYTE_REG(opcode)                                             \
static u32 _decode_##opcode##_reg(                                                \
    const u8 first_byte, const u8* mem, u32 size, Instruction* out)               \
{                                                                                 \
  u8 res=1;                                                                       \
  const Register reg = regs_dec[first_byte & 0x7][1];                             \
  UNUSED(mem);                                                                    \
  UNUSED(size);                                                                   \
  out->op = Opcode::opcode;                                                       \
  out->args[0].t = ArgReg;                                                        \
  out->args[0].reg = reg;                                                         \
  return res;                                                                     \
}

TEMPLATE_ONE_BYTE_REG(inc);
TEMPLATE_ONE_BYTE_REG(dec);


__attribute__((constructor))
static void _init_decoders_table(void) 
{
  for(u32 i=0; i<ArraySize(decoders); i++)
  {
    decoders[i] = _decode_invalid;
  }

  for(u8 i=0; i<=0b11; i++)
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

  decoders[0b10001110] = _decode_mov_seg_reg_to_reg_mem;
  decoders[0b10001100] = _decode_mov_reg_mem_to_seg_reg;

  for(u8 i=0; i<= 0b11; i++)
  {
    decoders[0b00000000 + i] = _decode_add_reg_mem_to_from_reg;
  }

  for(u8 i=0; i<= 0b11; i++)
  {
    decoders[0b00010000 + i] = _decode_adc_reg_mem_to_from_reg;
  }

  for(u8 i=0; i<= 0b11; i++)
  {
    decoders[0b10000000 + i] = _decode_arithm_imm_to_reg_mem;
  }

  for(u8 i=0; i<= 0b1; i++)
  {
    decoders[0b00000100 + i] = _decode_add_imm_to_acc;
  }

  for(u8 i=0; i<= 0b1; i++)
  {
    decoders[0b00010100 + i] = _decode_adc_imm_to_acc;
  }

  for(u8 i=0; i<= 0b11; i++)
  {
    decoders[0b00101000 + i] = _decode_sub_reg_mem_to_from_reg;
  }

  for(u8 i=0; i<= 0b11; i++)
  {
    decoders[0b00011000 + i] = _decode_sbb_reg_mem_to_from_reg;
  }

  for(u8 i=0; i<= 0b1; i++)
  {
    decoders[0b00101100 + i] = _decode_sub_imm_to_acc;
  }

  for(u8 i=0; i<= 0b1; i++)
  {
    decoders[0b00011100 + i] = _decode_sbb_imm_to_acc;
  }

  for(u8 i=0; i<= 0b11; i++)
  {
    decoders[0b00111000 + i] = _decode_cmp_reg_mem_to_from_reg;
  }

  for(u8 i=0; i<= 0b1; i++)
  {
    decoders[0b00111100 + i] = _decode_cmp_imm_to_acc;
  }

  for(u8 i=0; i<= 0b11; i++)
  {
    decoders[0b00100000 + i] = _decode_OpAnd_reg_mem_to_from_reg;
  }

  for(u8 i=0; i<= 0b1; i++)
  {
    decoders[0b00100100 + i] = _decode_OpAnd_imm_to_acc;
  }

  for(u8 i=0; i<= 0b11; i++)
  {
    decoders[0b10000100 + i] = _decode_test_reg_mem_to_from_reg; //HACK: again manual is wrong
  }

  for(u8 i=0; i<= 0b1; i++)
  {
    decoders[0b10101000 + i] = _decode_test_imm_to_acc;
  }

  for(u8 i=0; i<= 0b11; i++)
  {
    decoders[0b00001000 + i] = _decode_OpOr_reg_mem_to_from_reg;
  }

  for(u8 i=0; i<= 0b1; i++)
  {
    decoders[0b00001100 + i] = _decode_OpOr_imm_to_acc;
  }

  for(u8 i=0; i<= 0b11; i++)
  {
    decoders[0b00110000 + i] = _decode_OpXor_reg_mem_to_from_reg;
  }

  for(u8 i=0; i<= 0b1; i++)
  {
    decoders[0b00110100 + i] = _decode_OpXor_imm_to_acc;
  }

  for(u8 i=0; i<= 0b111; i++)
  {
    decoders[0b01010000 + i] = _decode_push_register;
  }

  for(u8 i=0; i<= 0b111; i++)
  {
    decoders[0b01011000 + i] = _decode_pop_register;
  }

  for(u8 i=0; i<= 0b11; i++)
  {
    decoders[0b00000111 | (i << 3)] = _decode_pop_segment_register;
  }

  for(u8 i=0; i<= 0b11; i++)
  {
    decoders[0b00000110 | (i << 3)] = _decode_push_segment_register;
  }

  decoders[0b10001111] = _decode_pop_reg_mem;

  for(u8 i=0; i<= 0b1; i++)
  {
    decoders[0b10000110 + i] = _decode_xchg_reg_mem_with_reg;
  }

  for(u8 i=0; i<= 0b111; i++)
  {
    decoders[0b10010000 + i] = _decode_xchg_reg_with_acc;
  }

  for(u8 i=0; i<= 0b1; i++)
  {
    decoders[0b11100100 + i] = _decode_in_fixed_port;
  }

  for(u8 i=0; i<= 0b1; i++)
  {
    decoders[0b11101100 + i] = _decode_in_variable_port;
  }

  for(u8 i=0; i<= 0b1; i++)
  {
    decoders[0b11100110 + i] = _decode_out_fixed_port;
  }

  for(u8 i=0; i<= 0b1; i++)
  {
    decoders[0b11101110 + i] = _decode_out_variable_port;
  }

  for(u8 i=0; i<= 0b111; i++)
  {
    decoders[0b01000000 | i] = _decode_inc_reg;
  }

  for(u8 i=0; i<= 0b111; i++)
  {
    decoders[0b01001000 | i] = _decode_dec_reg;
  }

  decoders[0b11010111] = _decode_xlat;

  decoders[0b10011000] = _decode_cbw;
  decoders[0b10011001] = _decode_cwd;

  decoders[0b10011111] = _decode_lahf;
  decoders[0b10011110] = _decode_sahf;
  decoders[0b10011100] = _decode_pushf;
  decoders[0b10011101] = _decode_popf;

  decoders[0b00110111] = _decode_aaa;
  decoders[0b00100111] = _decode_daa;

  decoders[0b00111111] = _decode_aas;
  decoders[0b00101111] = _decode_das;

  decoders[0b11010100] = _decode_aam;
  decoders[0b11010101] = _decode_aad;

  decoders[0b10001101] = _decode_lea_to_reg;
  decoders[0b11000101] = _decode_lds_to_reg;
  decoders[0b11000100] = _decode_les_to_reg;

  decoders[0b01110100] = _decode_je_jz;
  decoders[0b01111100] = _decode_jl_jnge;
  decoders[0b01111110] = _decode_jle_jng;
  decoders[0b01110010] = _decode_jb_jnae;
  decoders[0b01110110] = _decode_jbe_jna;
  decoders[0b01111010] = _decode_jp_jpe;
  decoders[0b01110000] = _decode_jo;
  decoders[0b01111000] = _decode_js;
  decoders[0b01110101] = _decode_jne_jnz;
  decoders[0b01111101] = _decode_jnl_jge;
  decoders[0b01111111] = _decode_jnle_jg;
  decoders[0b01110011] = _decode_jnb_jae;
  decoders[0b01110111] = _decode_jnbe_ja;
  decoders[0b01111011] = _decode_jnp_jpo;
  decoders[0b01110001] = _decode_jno;
  decoders[0b01111001] = _decode_jns;
  decoders[0b11100010] = _decode_loop;
  decoders[0b11100001] = _decode_loopz_loope;
  decoders[0b11100000] = _decode_loopnz_loopne;
  decoders[0b11100011] = _decode_jcxz;

  for(u8 i=0; i<= 0b1; i++)
  {
    decoders[0b11110110 | i] = _decode_neg_mul_div_imul_idiv_reg_mem;
  }

  for(u8 i=0; i<= 0b11; i++)
  {
    decoders[0b11010000 | i] = _decode_logic_shift_rotate;
  }

  decoders[0b11000011] = _decode_ret;
  decoders[0b11001011] = _decode_retf;
  decoders[0b11000010] = _decode_ret_XX_seg_add_imm_to_sp;
  decoders[0b11001010] = _decode_retf_XX_seg_add_imm_to_sp;

  for(u8 i=0; i<= 0b1; i++)
  {
    decoders[0b11110010 | i] = _decode_repeate;
  }

  decoders[0b11110000] = _decode_lock;
  for(u8 i=0; i<= 0b11; i++)
  {
    decoders[0b00100110 | (i << 3)] = _decode_seg_prefix;
  }

  decoders[0b11001101] = _decode_int_type_specified;
  decoders[0b11001100] = _decode_int3;
  decoders[0b11001110] = _decode_into;
  decoders[0b11001111] = _decode_iret;

  decoders[0b11111000] = _decode_clc;
  decoders[0b11110101] = _decode_cmc;
  decoders[0b11111001] = _decode_stc;
  decoders[0b11111100] = _decode_cld;
  decoders[0b11111101] = _decode_std;
  decoders[0b11111010] = _decode_cli;
  decoders[0b11111011] = _decode_sti;
  decoders[0b11110100] = _decode_hlt;
  decoders[0b10011011] = _decode_wait;

  decoders[0b10001100] = _decode_mov_reg_mem_to_seg_reg;

  decoders[0b10011010] =  _decode_call_direct_intersegment;
  decoders[0b11101010] =  _decode_jmp_direct_intersegment;
  decoders[0b11101011] =  _decode_jmp_direct_intersegment_short;
  decoders[0b11101000] =  _decode_call_direct_withinseg;
  decoders[0b11101001] =  _decode_jmp_direct_withinseg;

  decoders[0b11111110] = _decode_inc_dec_reg_mem;

  decoders[0b11111111] = _decode_indirect_intersegment;
}

u32 InstructionDecode(const u8* mem, const u32 mem_size, Instruction* out)
{
  u32 res = 0;
  u8 opcode;
  f_instruction_decoder decoder;

  if(!out) return -99;

  *out = {};
  out->seg = SegNone;

  if(mem_size < 1) return res;

  opcode = *mem;
  decoder = decoders[opcode];

  out->op = Opcode::INVALID;
  res = decoder(opcode, mem + 1, mem_size -1, out);
  out->size = res;

  switch (out->op)
  {
    case Opcode::mov: _cycles_mov(out); break;
    case Opcode::add: _cycles_add(out); break;
    default:
      fprintf(stderr, "cycles for opcode ");
      print_opcode(out->op, stderr);
      fprintf(stderr, " not implemented\n");
      break;
  }

  return res;
}
