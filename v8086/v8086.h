#pragma once

#include <string.h>

#include <v8086_definitions.h>

#include "memory/memory.h"
#include "decoder/decoder.h"

#define MAX_NUM_OF_PROGRAMS 32

typedef s32 ProgramID;

struct ProgramOptInfo{
  u32 cs_size = 64 * (1 << PowKilo);
  u32 ss_size = 64 * (1 << PowKilo);
  u32 ds_size = 64 * (1 << PowKilo);
  u32 es_size = 64 * (1 << PowKilo);
};

enum ProgSegments
{
  CS=0,
  SS,
  DS,
  ES,

  __ProgSegments_Count
};


struct ProgramSegment{
  LogicalSegment log_seg;
  u16 written;
};

struct Program
{
  ProgramSegment segment[__ProgSegments_Count];
  u32 decoding_index;
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
  return PhyMemoryIsValid(self.memory);
}

static inline int v8086RegSize(v8086& self)
{
  UNUSED(self);
  return 16;
}

ProgramID ProgramLoad(v8086& self, const char* file_program_path, const ProgramOptInfo& prog_info={});
  
int ProgramDumpNextInstr(v8086& self,const ProgramID prog_id, Instruction* out);

static inline void v8086Shutdown(v8086& self)
{
  PhyMemoryFree(self.memory);
  memset(self.running, 0, sizeof(self.running));
  self.next_free =0;
}
