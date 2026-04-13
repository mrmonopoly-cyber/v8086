#pragma once

#include <v8086_definitions.h>

struct PhyMemory{
  u8* Memory;
  u32 mask;
};

PhyMemory PhyMemoryAllocate(u32 capacity = (1 << PowMem::PowMega));

static inline bool PhyMemoryIsValid(PhyMemory& mem)
{
  return mem.mask != 0;
}

static inline u32 PhyMemoryCapacity(PhyMemory& mem)
{
  return mem.mask + 1;
}

static inline u8* PhyGetAddrAt(PhyMemory& mem, u32 addr)
{
  u8* res = nullptr;

  res = &mem.Memory[addr & mem.mask];

  return res;
}

void PhyMemoryFree(PhyMemory& mem);
