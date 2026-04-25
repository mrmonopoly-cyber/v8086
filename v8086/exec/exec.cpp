#include "exec.h"
#include "cpu.h"

#include <assert.h>
#include <cstdio>
#include <string.h>

#include <v8086_definitions.h>

typedef union
{
  s8 _s8;
  s16 _s16;
}NumConv;

typedef s32 (*op_exec)(Instruction* instr, CPU* cpu, SegmentView segmens[__Num_Segment]);

static u8 dummy_u8_buffer;
static op_exec executor [(size_t)Opcode::__Opcode_count];

static inline u8* _reg_to_ptr(Register reg, CPU* cpu, u8* o_reg_size = &dummy_u8_buffer)
{
  u8* res = nullptr;

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

  assert(res);

  return res;
}

static inline u8* _arg_to_ptr(
    Arg* arg,
    CPU* cpu,
    SegmentView segments[__Num_Segment],
    u8* o_reg_size = &dummy_u8_buffer)
{
  u8* res = nullptr;

  switch (arg->t)
  {
    case ArgInvalid:
      TODO();
      break;
    case ArgReg:
      res = _reg_to_ptr(arg->reg, cpu, o_reg_size);
      break;
    case ArgMem:
      TODO();
      break;
    case ArgUImm8:
      res = &arg->uimm8;
      *o_reg_size = 1;
      break;
    case ArgUImm16:
      res = (u8*) &arg->uimm16;
      *o_reg_size = 2;
      break;
    case ArgImm8:
      res = (u8*) &arg->imm8;
      *o_reg_size = 1;
      break;
    case ArgImm16:
      res = (u8*) &arg->imm16;
      *o_reg_size = 2;
      break;
    case ArgMemRegDisp:
      TODO();
      break;
    case ArgMemRegRegDisp:
      TODO();
      break;
    case ArgSegment:
      res = segments[arg->seg].data;
      break;
    case ArgDirInterSeg:
      TODO();
      break;
    case ArgIpInc8:
      TODO();
      break;
    case ArgIpInc16:
      TODO();
      break;
  }

  assert(res);

  return res;
}

static s32 invalid_op_exec(Instruction* instr, CPU* cpu, SegmentView segmens[__Num_Segment])
{
  s32 res=-1;

  UNUSED(cpu);
  UNUSED(instr);
  UNUSED(segmens);

  return res;
}

static s32 _exec_mov(Instruction* instr, CPU* cpu, SegmentView segmens[__Num_Segment])
{
  s32 res=0;
  u8* src, *dst;
  u8 byte_to_move = 1;

  dst = _arg_to_ptr(&instr->args[0], cpu, segmens, &byte_to_move);
  src = _arg_to_ptr(&instr->args[1], cpu, segmens);

  cpu->ip += instr->size;

  memcpy(dst, src, byte_to_move);

  return res;
}

//INFO: add updates AF, CF, OF, PF, FS, ZF
static s32 _exec_add(Instruction* instr, CPU* cpu, SegmentView segmens[__Num_Segment])
{
  s32 res=0;
  u8* src, *dst;
  u8 byte_to_move = 1;
  NumConv num_s={0}, num_d={0}, old_v;
  u8 sf;
  u8 cf;

  cpu->ip += instr->size;
  CPU_flag_clear(cpu, (1<<OF) | (1<<SF) | (1<<CF) | (1<<ZF));

  dst = _arg_to_ptr(&instr->args[0], cpu, segmens, &byte_to_move);
  src = _arg_to_ptr(&instr->args[1], cpu, segmens);

  assert(byte_to_move >=1 && byte_to_move <= 2);

  memcpy(&num_s._s16, src, byte_to_move);
  memcpy(&num_d._s16, dst, byte_to_move);

  old_v = num_d;

  switch (byte_to_move)
  {
    case 0x1:
      num_d._s8 += num_s._s8;
      sf = num_d._s8 < 0;
      cf = old_v._s8 >=0 && num_d._s8 < 0;
      if(num_d._s8 >0 && num_s._s8 > 0 && old_v._s8 <0 && num_d._s8 < 0)
      {
        CPU_flag_set(cpu, 1 << OF);
      }
      break;
    case 0x2:
      num_d._s16 += num_s._s16;
      sf = num_d._s16 < 0;
      cf = old_v._s16 >=0 && num_d._s16 < 0;
      if(num_d._s16 >0 && num_s._s16 > 0 && old_v._s16 <0 && num_d._s16 < 0)
      {
        CPU_flag_set(cpu, 1 << OF);
      }
      break;
  }

  memcpy(dst, &num_d, byte_to_move);
  CPU_flag_set(cpu, (sf << SF) | (cf << CF) | (!(num_d._s16) << ZF));

  return res;
}

//INFO: sub updates AF, CF, OF, PF, FS, ZF
static s32 _exec_sub(Instruction* instr, CPU* cpu, SegmentView segmens[__Num_Segment])
{
  s32 res=0;
  u8* src, *dst;
  u8 byte_to_move = 1;
  NumConv num_s={0}, num_d={0}, old_v;
  u8 sf;
  u8 cf;

  cpu->ip += instr->size;
  CPU_flag_clear(cpu, (1<<OF) | (1<<SF) | (1<<CF) | (1<<ZF));

  dst = _arg_to_ptr(&instr->args[0], cpu, segmens, &byte_to_move);
  src = _arg_to_ptr(&instr->args[1], cpu, segmens);

  assert(byte_to_move >=1 && byte_to_move <= 2);

  memcpy(&num_s._s16, src, byte_to_move);
  memcpy(&num_d._s16, dst, byte_to_move);

  old_v = num_d;

  switch (byte_to_move)
  {
    case 0x1:
      num_d._s8 -= num_s._s8;
      sf = num_d._s8 < 0;
      cf = old_v._s8 >=0 && num_d._s8 < 0;
      break;
    case 0x2:
      num_d._s16 -= num_s._s16;
      sf = num_d._s16 < 0;
      cf = old_v._s16 >=0 && num_d._s16 < 0;
      break;
  }

  memcpy(dst, &num_d, byte_to_move);
  CPU_flag_set(cpu, (sf << SF) | (cf << CF) | (!(num_d._s16) << ZF));

  return res;
}

//INFO: cmp updates AF, CF, OF, PF, FS, ZF
static s32 _exec_cmp(Instruction* instr, CPU* cpu, SegmentView segmens[__Num_Segment])
{
  s32 res=0;
  u8* src, *dst;
  u8 byte_to_move = 1;
  NumConv num_s={0}, num_d={0}, old_v;
  u8 sf;
  u8 cf;

  cpu->ip += instr->size;
  CPU_flag_clear(cpu, (1<<OF) | (1<<SF) | (1<<CF) | (1<<ZF));

  dst = _arg_to_ptr(&instr->args[0], cpu, segmens, &byte_to_move);
  src = _arg_to_ptr(&instr->args[1], cpu, segmens);

  assert(byte_to_move >=1 && byte_to_move <= 2);

  memcpy(&num_s._s16, src, byte_to_move);
  memcpy(&num_d._s16, dst, byte_to_move);

  old_v = num_d;

  switch (byte_to_move)
  {
    case 0x1:
      num_d._s8 -= num_s._s8;
      sf = num_d._s8 < 0;
      cf = old_v._s8 >=0 && num_d._s8 < 0;
      break;
    case 0x2:
      num_d._s16 -= num_s._s16;
      sf = num_d._s16 < 0;
      cf = old_v._s16 >=0 && num_d._s16 < 0;
      break;
  }

  CPU_flag_set(cpu, (sf << SF) | (cf << CF) | (!(num_d._s16) << ZF));

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
  executor[(size_t)Opcode::sub] = _exec_sub;
  executor[(size_t)Opcode::add] = _exec_add;
  executor[(size_t)Opcode::cmp] = _exec_cmp;
}

s32 InstructionExec(Instruction* const instr, CPU* cpu, SegmentView segmens[__Num_Segment])
{
  s32 res=0;
  op_exec exec = executor[(size_t)instr->op];

  res = exec(instr, cpu, segmens);

  return res;
}
