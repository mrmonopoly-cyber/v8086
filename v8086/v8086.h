#pragma once

#include "v8086_definitions.h"

#include "memory.h"
#include "decoder.h"

#define MAX_NUM_OF_PROGRAMS 32

typedef s32 ProgramID;

struct ProgramInfo{
  u16 cs_size = 16 * (1 << PowKilo);
  u16 ss_size = 16 * (1 << PowKilo);
  u16 ds_size = 16 * (1 << PowKilo);
  u16 es_size = 16 * (1 << PowKilo);
  const char* file_program_path = nullptr;
};

struct Program
{
  LogicalSegment cs;
  LogicalSegment ss;
  LogicalSegment ds;
  LogicalSegment es;
};

struct v8086
{
  PhyMemory memory;
  Program running[MAX_NUM_OF_PROGRAMS];
  u32 next_free=0;
};

static inline int v8086PowerOn(v8086& self)
{
  self.memory = PhyMemoryAllocate();
  return -PhyMemoryIsValid(self.memory);
}

ProgramID loadProgram(v8086& self, const ProgramInfo& prog_info);
u32 programLength(const v8086& self, const ProgramID prog);
Instruction programDecodeInstrAt(const v8086& self,const ProgramID prog, const u32 offset);

static inline void v8086Shutdown(v8086& self)
{
  PhyMemoryFree(self.memory);
}
