#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "cpu.h"

#define ArraySize(Array) (sizeof(Array)/sizeof(Array[0]))

enum PowMem : u32{
  PowByte = 1,
  PowKilo = 10 * PowByte,
  PowMega = 2 * PowKilo,
  PowGiga = 3 * PowKilo,
};

#define OPS \
  X(mov)\
  X(add)\
  X(adc)\
  X(sub)\
  X(sbb)\
  X(cmp)\
  X(jz)\
  X(jl)\
  X(jle)\
  X(jb)\
  X(jbe)\
  X(jp)\
  X(jo)\
  X(js)\
  X(jnz)\
  X(jnl)\
  X(jnle)\
  X(jnb)\
  X(jnbe)\
  X(jnp)\
  X(jno)\
  X(jns)\
  X(loop)\
  X(loopz)\
  X(loopnz)\
  X(jcxz)\
  X(call)\
  X(jmp)\
  X(push)\
  X(pop)\
  X(xchg)\
  X(in)\
  X(out)\
  X(xlat)\
  X(lea)\
  X(lds)\
  X(les)\
  X(lahf)\
  X(sahf)\
  X(pushf)\
  X(popf)\
  X(inc)\
  X(dec)\
  X(aaa)\
  X(daa)\
  X(neg)\
  X(aas)\
  X(das)\
  X(mul)\
  X(imul)\
  X(div)\
  X(idiv)\
  X(aam)\
  X(aad)\
  X(cbw)\
  X(cwd)\
  X(rol)\
  X(ror)\
  X(rcl)\
  X(rcr)\
  X(shl)\
  X(shr)\
  X(sar)\
  X(test)\
  X(movsb)\
  X(cmpsb)\
  X(scasb)\
  X(lodsb)\
  X(stosb)\
  X(movsw)\
  X(cmpsw)\
  X(scasw)\
  X(lodsw)\
  X(stosw)\
  X(ret)\
  X(retf)\
  X(into)\
  X(iret)\
  X(int3)\
  X(clc)\
  X(cmc)\
  X(stc)\
  X(cld)\
  X(std)\
  X(cli)\
  X(sti)\
  X(hlt)\
  X(wait)\

enum class Opcode
{
  INVALID,
  OpNot,
  OpAnd,
  OpOr,
  OpXor,
  OpInt,
#define X(OP) OP,
  OPS
#undef X

  __Opcode_count
};

#define PREFIXES \
  X(rep)\
  X(lock)\

enum class Prefix
{
  None,
#define X(pr) pr,
  PREFIXES
#undef X
};

enum Segment{
  SegNone = -1,
  ES = 0b00,
  CS = 0b01,
  SS = 0b10,
  DS = 0b11,

  __Num_Segment,
};

enum ArgType
{
  ArgInvalid,
  ArgReg,
  ArgMem,
  ArgUImm8,
  ArgUImm16,
  ArgImm8,
  ArgImm16,
  ArgMemRegDisp,
  ArgMemRegRegDisp,
  ArgSegment,
  ArgDirInterSeg,
  ArgIpInc8,
  ArgIpInc16,
};

struct MemAddr{
  u32 addr;
  u8 word;
};

enum DispType
{
  Disp8,
  Disp16,
};

struct Displacement{
  DispType t;
  union{
    s8 disp8;
    s16 disp16;
  };
  u8 word;
  u8 far;
};

#define REGS\
  X(al)\
  X(ax)\
  X(cl)\
  X(cx)\
  X(dl)\
  X(dx)\
  X(bl)\
  X(bx)\
  X(ah)\
  X(sp)\
  X(ch)\
  X(bp)\
  X(dh)\
  X(si)\
  X(bh)\
  X(di)\

enum Register : u16
{
  INVALID_REG,
#define X(r) dec_##r,
  REGS
#undef X
};

struct RegDisp{
  Register r1;
  Displacement disp;
};

struct RegRegDisp{
  Register r1;
  Register r2;
  Displacement disp;
};

struct DirectIntersegmen{
  u16 addr[2];
};

struct Arg{
  ArgType t;
  union{
    Register reg;
    Segment seg;
    MemAddr addr;
    u8 uimm8;
    u16 uimm16;
    s8 imm8;
    s16 imm16;
    RegDisp reg_disp;
    RegRegDisp reg_reg_disp;
    DirectIntersegmen dir_inter_seg;
    s8 ip_inc_8;
    s16 ip_inc_16;
  };
};

struct Instruction{
  Prefix prefix;
  Segment seg;
  Opcode op;
  u16 size;
  Arg args[2];
};

template<typename T>
static inline void _print_byte(T byte, FILE* out)
{
  fprintf(out, "0b");
  const size_t num_bits = 8 * sizeof(byte);

  for(u8 i=0; i<num_bits; i++)
  {
    if((byte>> ((num_bits-1) - i)) & 0x1)
    {
      fprintf(out, "1");
    }else
    {
      fprintf(out, "0");
    }
  }
  fprintf(out, " (%d)", byte);
}

void print_seg(const Segment seg, FILE* out = stdout);
void print_disp(const Displacement* disp, FILE *out = stdout);
void print_reg(const Register reg, FILE* out = stdout);
void print_arg(const Arg* const arg, FILE* out, const Segment seg);
void InstructionPrint(const Instruction& instr, FILE* out_f = stdout);
void CPUPrint(CPU* cpu, FILE* out = stdout);

#define TODO(...)\
  do{printf("%s.%d: TODO: %s\n", __FILE__, __LINE__, __VA_ARGS__""); exit(99);}while(0);

#define UNUSED(X) (void)X
