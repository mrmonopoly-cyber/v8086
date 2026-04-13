#include "memory.h"

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
