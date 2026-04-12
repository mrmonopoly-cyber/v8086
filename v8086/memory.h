#pragma once

#include <stdlib.h>

#include "v8086_definitions.h"

#define FREE_POOL_SIZE 32

struct MemBlock{
  u8* base;
  u32 capacity;
  u32 next;
  u32 in_use;
};

struct FreeList{
  MemBlock free_list_pool[FREE_POOL_SIZE];
  u32 free_index_queue[FREE_POOL_SIZE];
  s32 next_free_index;
  s32 root;
};

struct PhyMemory{
  u8* Memory;
  u32 capacity;
  u32 mask;
  FreeList free_list;
};

struct LogicalSegment{
  u16 segment_base;
  u16 segment_offset;
  u32 written;
};

PhyMemory PhyMemoryAllocate();

LogicalSegment PhyMemorySaveSegment(PhyMemory& self, const u32 size);

u8* PhyMemoryGetAddressOf(PhyMemory& self, LogicalSegment& seg);

static inline bool PhyMemoryIsValid(PhyMemory& mem)
{
  return mem.mask != 0;
}

static inline void PhyMemoryFree(PhyMemory& mem)
{
  if(PhyMemoryIsValid(mem)){
    free(mem.Memory);
  }
}
