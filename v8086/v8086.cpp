#include "v8086.h"

#include <stdio.h>

#include "memory.h"

static inline ProgramID next_prog_id(v8086& self)
{
  if(self.next_free < self.memory.capacity)
  {
    return self.next_free++;
  }

  return -1;
}

ProgramID loadProgram(v8086& self, const ProgramInfo& prog_info)
{
  ProgramID res = -1;
  Program* prog = nullptr;
  FILE* program_f = fopen(prog_info.file_program_path, "rb");
  u8* phy_addr = nullptr;

  if(program_f == nullptr){
    return res;
  }

  res = next_prog_id(self);

  if(res >= 0)
  {
    prog = &self.running[res];
    prog->cs = PhyMemorySaveSegment(self.memory, prog_info.cs_size);
    prog->ss = PhyMemorySaveSegment(self.memory, prog_info.ss_size);
    prog->ds = PhyMemorySaveSegment(self.memory, prog_info.ds_size);
    prog->es = PhyMemorySaveSegment(self.memory, prog_info.es_size);

    if(
        !prog->cs.segment_base || !prog->ss.segment_base ||
        !prog->ds.segment_base || !prog->es.segment_base
      )
    {
      self.next_free--;
      res = -1;
      return res;
    }

    phy_addr = PhyMemoryGetAddressOf(self.memory, prog->cs);
    prog->cs.written = fread(phy_addr,1, prog_info.cs_size, program_f);
  }

  return res;
}

u32 programLength(const v8086& self, const ProgramID prog)
{
  const Program* p_prog = &self.running[prog];

  return p_prog->cs.written;
}

Instruction programDecodeInstrAt(const v8086& self,const ProgramID prog, const u32 offset)
{

  return {};
}
