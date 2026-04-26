#include "exec.h"
#include "cpu.h"

#include <assert.h>
#include <string.h>
#include <limits.h>

#include <v8086_definitions.h>

enum ConvType
{
  S8=1,
  S16,
  S32,
};

typedef struct
{
  ConvType t;
  union{
    s8 _s8;
    s16 _s16;
    s32 _s32;
  };
}NumConv;

typedef s32 (*op_exec)(Instruction* instr, CPU* cpu, SegmentView segmens[__Num_Segment],
    u16* old_val, u16* new_val);

static u8 dummy_u8_buffer;
static op_exec executor [(size_t)Opcode::__Opcode_count];

static inline void _check_set_parity_flag(CPU* cpu, NumConv val)
{
  u8 num_1=0;
  u8 cval = val._s32;
  flag_clear(&cpu->flags, 1 << PF);

  for(u8 i=0; i<val.t* 8; i++)
  {
    num_1 += ((cval >> i) & 0x1);
  }

  if(!(num_1 & 0x1)) //even
  {
    flag_set(&cpu->flags, 1 << PF);
  }
}

static inline void _check_set_overflow_flag(CPU* cpu, NumConv val)
{
  const u8 condition = 
    ((val.t == ConvType::S8) && ((val._s32 < INT8_MIN) || (val._s32 > INT8_MAX))) ||
    ((val.t == ConvType::S16) && ((val._s32 < INT16_MIN ) || (val._s32 > INT16_MAX)));

  flag_clear(&cpu->flags, 1u << OF);
  flag_set(&cpu->flags, condition << OF);
}

static inline void _check_set_sign_flag(CPU* cpu, NumConv val)
{
  flag_clear(&cpu->flags, 1 << SF);
  const u8 condition = 
    ((val.t == ConvType::S8) && (((s8)val._s32) < 0)) ||
    ((val.t == ConvType::S16) && (((s16)val._s32) < 0));

  flag_set(&cpu->flags, condition << SF);
}

static inline void _check_set_zero_flag(CPU* cpu, NumConv val)
{
  flag_clear(&cpu->flags, 1 << ZF);
  flag_set(&cpu->flags, (!val._s32) << ZF);
}

static inline void _check_all_flags(CPU* cpu, NumConv val)
{
  _check_set_parity_flag(cpu, val);
  _check_set_overflow_flag(cpu, val);
  _check_set_sign_flag(cpu, val);
  _check_set_zero_flag(cpu, val);
}

static inline void _store_sized_value(NumConv *cval, void* data)
{
  switch (cval->t)
  {
    case S8:
      cval->_s32 = *(s8*)data;
      break;
    case S16:
      cval->_s32 = *(s16*)data;
      break;
    case S32:
      cval->_s32 = *(s32*)data;
      break;
  }
}

static inline void _load_sized_value(NumConv *cval, void* data)
{

  switch (cval->t)
  {
    case S8:
      *((u8*)data) = cval->_s32;
      break;
    case S16:
      *((u16*)data) = cval->_s32;
      break;
    case S32:
      *((u32*)data) = cval->_s32;
      break;
  }
}

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

static s32 invalid_op_exec(Instruction* instr, CPU* cpu, SegmentView segmens[__Num_Segment],
    u16* old_val, u16* new_val)
{
  s32 res=-1;

  UNUSED(cpu);
  UNUSED(instr);
  UNUSED(segmens);
  UNUSED(old_val);
  UNUSED(new_val);

  return res;
}

static s32 _exec_mov(Instruction* instr, CPU* cpu, SegmentView segmens[__Num_Segment],
    u16* old_val, u16* new_val)
{
  s32 res=0;
  u8* src, *dst;
  u8 byte_to_move = 1;

  dst = _arg_to_ptr(&instr->args[0], cpu, segmens, &byte_to_move);
  src = _arg_to_ptr(&instr->args[1], cpu, segmens);

  cpu->ip += instr->size;

  memcpy(old_val, dst, byte_to_move);
  memcpy(dst, src, byte_to_move);
  memcpy(new_val, dst, byte_to_move);

  return res;
}

//INFO: add updates AF, CF, OF, PF, FS, ZF
static s32 _exec_add(Instruction* instr, CPU* cpu, SegmentView segmens[__Num_Segment],
    u16* old_val, u16* new_val)
{
  s32 res=0;
  u8* src, *dst;
  NumConv num_s={}, num_d={};

  cpu->ip += instr->size;

  dst = _arg_to_ptr(&instr->args[0], cpu, segmens, (u8*) &num_d.t);
  src = _arg_to_ptr(&instr->args[1], cpu, segmens, (u8*) &num_s.t);

  _store_sized_value(&num_s, src);
  _store_sized_value(&num_d, dst);

  *old_val = num_d._s16;
  num_d._s32 += num_s._s32;
  *new_val = num_d._s16;

  _load_sized_value(&num_d, dst);
  _check_all_flags(cpu, num_d);

  return res;
}

//INFO: sub updates AF, CF, OF, PF, FS, ZF
static s32 _exec_sub(Instruction* instr, CPU* cpu, SegmentView segmens[__Num_Segment],
    u16* old_val, u16* new_val)
{
  s32 res=0;
  u8* src, *dst;
  NumConv num_s={}, num_d={};

  cpu->ip += instr->size;

  dst = _arg_to_ptr(&instr->args[0], cpu, segmens, (u8*) &num_d.t);
  src = _arg_to_ptr(&instr->args[1], cpu, segmens, (u8*) &num_s.t);

  _store_sized_value(&num_s, src);
  _store_sized_value(&num_d, dst);

  *old_val = num_d._s32;
  num_d._s32 -= num_s._s32;
  *new_val = num_d._s32;

  _load_sized_value(&num_d, dst);
  _check_all_flags(cpu, num_d);

  return res;
}

//INFO: cmp updates AF, CF, OF, PF, FS, ZF
static s32 _exec_cmp(Instruction* instr, CPU* cpu, SegmentView segmens[__Num_Segment],
    u16* old_val, u16* new_val)
{
  s32 res=0;
  u8* src, *dst;
  NumConv num_s={}, num_d={};

  UNUSED(old_val);
  UNUSED(new_val);

  cpu->ip += instr->size;

  dst = _arg_to_ptr(&instr->args[0], cpu, segmens, (u8*) &num_d.t);
  src = _arg_to_ptr(&instr->args[1], cpu, segmens, (u8*) &num_s.t);

  _store_sized_value(&num_s, src);
  _store_sized_value(&num_d, dst);

  num_d._s32 -= num_s._s32;
  _check_all_flags(cpu, num_d);

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

s32 InstructionExec(Instruction* const instr, CPU* cpu, SegmentView segmens[__Num_Segment],
    u16* old_val, u16* new_val)
{
  s32 res=0;
  op_exec exec = executor[(size_t)instr->op];

  *old_val = 0;
  *new_val = 0;

  res = exec(instr, cpu, segmens, old_val, new_val);

  return res;
}
