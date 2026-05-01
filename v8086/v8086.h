#pragma once

#include <v8086_definitions.h>

#include "memory/memory.h"

#define MAX_NUM_OF_PROGRAMS 32

typedef s32 ProgramID;

struct ProgramOptInfo{
  u32 cs_size = 64 * (1 << PowKilo);
  u32 ss_size = 64 * (1 << PowKilo);
  u32 ds_size = 64 * (1 << PowKilo);
  u32 es_size = 64 * (1 << PowKilo);
};

struct ProgramSegment{
  LogicalSegment log_seg;
  u16 written;
};

struct Program{
  ProgramSegment segment[__Num_Segment];
  u32 decoding_index;
};

struct v8086
{
  PhyMemory memory;
  CPU cpu;
  Program running[MAX_NUM_OF_PROGRAMS];
  u32 next_free=0;
};

static inline int v8086PowerOn(v8086& self)
{
  self.memory = PhyMemoryAllocate();
  return PhyMemoryIsValid(self.memory);
}

static inline int v8086RegSize(v8086& self)
{
  UNUSED(self);
  return 16;
}

ProgramID ProgramLoad(v8086& self, const char* file_program_path, const ProgramOptInfo& prog_info={});
  
int ProgramDumpNextInstr(v8086& self,const ProgramID prog_id, Instruction* out);
void V8086Dump(v8086& self, ProgramID prog_id =-1, FILE* out=stdout);

size_t V8086DumpSegment(v8086& self, ProgramID prog_id, Segment segment, FILE* out=stdout);

enum RunMode
{
  Normal,
  Debug
};

int ProgramRun(v8086& self, ProgramID prog_id, FILE* out, RunMode mode = RunMode::Normal);

static inline void v8086Shutdown(v8086& self)
{
  PhyMemoryFree(self.memory);
  self.next_free =0;
}
