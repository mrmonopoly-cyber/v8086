#include "memory.h"
#include "v8086_definitions.h"

#include <cassert>
#include <cstdio>
#include <stdlib.h>


LogicalSegment MakeSegment(PhyMemory& mem, u32 addr)
{
  LogicalSegment res;

  res.mask = mem.mask;
  addr &= res.mask;

  res.segment_base = addr >> 4;
  res.segment_offset = addr & 0xF;

  return res;
}

void SegmentPrint(LogicalSegment* seg, PhyMemory* mem, FILE* out, u32 amount)
{
  u8* mem_ptr = nullptr;
  u32 size;
  u32 physical_addr = 0;
  assert(seg);
  assert(mem);

  if(out == nullptr) out = stdout;
  size = seg->mask+1;

  for(size_t i=0; i<size && i<amount; i++)
  {
    physical_addr = AddrFromSegment(*seg, i);
    mem_ptr = PhyGetAddrAt(*mem, physical_addr);
    fprintf(out, "0x%zx:", i);
    _print_byte(*mem_ptr, out);
    fprintf(out, "\n");
  }
}
