#pragma once

#include <v8086_definitions.h>

struct SegmentView{
  u8* data;
  u8 len;
};

struct ExecUpdate{
  u16* old_val;
  u16* new_val;
  u8 flags;
};

s32 InstructionExec(Instruction* const instr, CPU* cpu, SegmentView segmens[__Num_Segment],
    u16* old_val, u16* new_val);
