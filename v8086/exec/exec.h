#pragma once

#include <v8086_definitions.h>

struct SegmentView{
  u8* data;
  u8 len;
};

s32 InstructionExec(Instruction* const instr, CPU* cpu, SegmentView segmens[__Num_Segment]);
