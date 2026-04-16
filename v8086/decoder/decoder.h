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

enum Segment{
  ES = 0b00,
  CS = 0b01,
  SS = 0b10,
  DS = 0b11,
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
    Segment seg;
    u32 addr;
    u8 uimm8;
    u16 uimm16;
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
