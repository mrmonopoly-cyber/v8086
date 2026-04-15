#pragma once

#include <stdio.h>

#include <v8086_definitions.h>

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

enum Opcode
{
#define X(OP) OP,
  OPS
#undef X
  INVALID
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
  X(di)

enum Register : u16
{
  INVALID_REG,
#define X(r) r,
  REGS
#undef X
};

enum ArgType
{
  ArgInvalid,
  ArgReg,
  ArgMem,
  ArgImm8,
  ArgImm16,
  ArgMemRegDisp,
  ArgMemRegRegDisp,
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

struct Arg{
  ArgType t;
  union{
    Register reg;
    u32 addr;
    s8 imm8;
    s16 imm16;
    RegDisp reg_disp;
    RegRegDisp reg_reg_disp;
  };
};

struct Instruction{
  Opcode op;
  Arg args[2];
};

void InstructionPrint(const Instruction& instr, FILE* out);

u32 InstructionDecode(const u8* mem, const u32 mem_size, Instruction* const out);
