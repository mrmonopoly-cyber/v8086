#include "exec.h"

#include <cstring>
#include <v8086_definitions.h>

#include <assert.h>

typedef s32 (*op_exec)(Instruction* instr, CPU* cpu);

static op_exec executor [(size_t)Opcode::__Opcode_count];

static inline u8* _reg_to_ptr(Register reg, CPU* cpu, u8* o_reg_size = nullptr)
{
  u8* res = nullptr;
  u8 o_reg_size_fallback;
  if(o_reg_size == nullptr) o_reg_size = &o_reg_size_fallback;

  switch(reg)
  {
    case INVALID_REG:
      assert(0 && "invalid reg");
      break;
    case dec_ax:
      res = (u8*) &cpu->regs[ax]._u16;
      *o_reg_size = 2;
      break;
    case dec_al:
      res = &cpu->regs[ax]._half.l;
      *o_reg_size = 1;
      break;
    case dec_ah:
      res = &cpu->regs[ax]._half.h;
      *o_reg_size = 1;
      break;

    case dec_bx:
      res = (u8*) &cpu->regs[bx]._u16;
      *o_reg_size = 2;
      break;
    case dec_bl:
      res = &cpu->regs[bx]._half.l;
      *o_reg_size = 1;
      break;
    case dec_bh:
      res = &cpu->regs[bx]._half.h;
      *o_reg_size = 1;
      break;

    case dec_cx:
      res = (u8*) &cpu->regs[cx]._u16;
      *o_reg_size = 2;
      break;
    case dec_cl:
      res = &cpu->regs[cx]._half.l;
      *o_reg_size = 1;
      break;
    case dec_ch:
      res = &cpu->regs[cx]._half.h;
      *o_reg_size = 1;
      break;

    case dec_dx:
      res = (u8*) &cpu->regs[dx]._u16;
      *o_reg_size = 2;
      break;
    case dec_dl:
      res = &cpu->regs[dx]._half.l;
      *o_reg_size = 1;
      break;
    case dec_dh:
      res = &cpu->regs[dx]._half.h;
      *o_reg_size = 1;
      break;

    case dec_si:
      res = (u8*) &cpu->regs[si]._u16;
      *o_reg_size = 2;
      break;
    case dec_di:
      res = (u8*) &cpu->regs[di]._u16;
      *o_reg_size = 2;
      break;
    case dec_sp:
      res = (u8*) &cpu->regs[sp]._u16;
      *o_reg_size = 2;
      break;
    case dec_bp:
      res = (u8*) &cpu->regs[bp]._u16;
      *o_reg_size = 2;
      break;
  }

  return res;
}

static s32 _exec_mov(Instruction* instr, CPU* cpu)
{
  s32 res=0;
  u8* src, *dst;
  u8 byte_to_move = 1;

  switch (instr->args[0].t)
  {
    case ArgInvalid:
      assert(0 && "mov first arg cannot be Invalid in mov");
      break;
    case ArgReg:
      dst = _reg_to_ptr(instr->args[0].reg, cpu, &byte_to_move);
      break;
    case ArgMem:
      TODO();
      break;
    case ArgUImm8:
      assert(0 && "mov first arg cannot be uimm8");
      break;
    case ArgUImm16:
      assert(0 && "mov first arg cannot be uimm16");
      break;
    case ArgImm8:
      assert(0 && "mov first arg cannot be imm8");
      break;
    case ArgImm16:
      assert(0 && "mov first arg cannot be imm16");
      break;
    case ArgMemRegDisp:
      TODO();
      break;
    case ArgMemRegRegDisp:
      TODO();
      break;
    case ArgSegment:
      TODO();
      break;
    case ArgDirInterSeg:
      TODO();
      break;
    case ArgIpInc8:
      assert(0 && "mov first arg cannot be ip inc 8");
      break;
    case ArgIpInc16:
      assert(0 && "mov first arg cannot be ip inc 16");
      break;
  }

  switch (instr->args[1].t)
  {
    case ArgInvalid:
      assert(0 && "mov first arg cannot be Invalid in mov");
      break;
    case ArgReg:
      src = _reg_to_ptr(instr->args[0].reg, cpu);
      break;
    case ArgMem:
      TODO();
      break;
    case ArgUImm8:
      src = &instr->args[1].uimm8;
      break;
    case ArgUImm16:
      src = (u8*) &instr->args[1].uimm16;
      break;
    case ArgImm8:
      src = (u8*) &instr->args[1].imm8;
      break;
    case ArgImm16:
      src = (u8*) &instr->args[1].uimm16;
      break;
    case ArgMemRegDisp:
      TODO();
      break;
    case ArgMemRegRegDisp:
      TODO();
      break;
    case ArgSegment:
      TODO();
      break;
    case ArgDirInterSeg:
      TODO();
      break;
    case ArgIpInc8:
      assert(0 && "mov first arg cannot be ip inc 8");
      break;
    case ArgIpInc16:
      assert(0 && "mov first arg cannot be ip inc 16");
      break;
  }

  memcpy(dst, src, byte_to_move);

  return res;
}

s32 invalid_op_exec(Instruction* instr, CPU* cpu)
{
  s32 res=-1;

  UNUSED(cpu);
  UNUSED(instr);

  return res;
}

__attribute__((constructor))
static void init_executor_array(void)
{
  for(size_t i=0; i<ArraySize(executor); i++)
  {
    executor[i] = invalid_op_exec;
  }

  executor[(size_t)Opcode::mov] = _exec_mov;
}

s32 InstructionExec(Instruction* const instr, CPU* cpu)
{
  s32 res=0;
  op_exec exec = executor[(size_t)instr->op];

  res = exec(instr, cpu);

  return res;
}
