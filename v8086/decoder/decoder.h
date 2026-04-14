#pragma once

#include <stdio.h>

#include <v8086_definitions.h>

#define OPS \
  X(mov)

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
  ArgReg,
  ArgMem,
  ArgImm8,
  ArgImm16,
  ArgMemRegDisp,
  ArgMemRegRegDisp,
};

struct RegDisp{
  Register r1;
  u16 disp;
};

struct RegRegDisp{
  Register r1;
  Register r2;
  u16 disp;
};

struct Arg{
  ArgType t;
  union{
    Register reg;
    u32 addr;
    u8 imm8;
    u16 imm16;
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
