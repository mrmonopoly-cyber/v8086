#pragma once

#include <v8086_definitions.h>

#include "physical.h"

struct LogicalSegment{
  u16 segment_base;
  u16 segment_offset;
  u32 mask;
};

LogicalSegment MakeSegment(PhyMemory& mem, u32 addr);

static inline u32 AddrFromSegment(const LogicalSegment& seg, const u16 offset=0)
{
  u32 res=0;
  res = (((u32)seg.segment_base << 4) + ((u32) seg.segment_offset + offset)) & seg.mask;
  return res;
}

