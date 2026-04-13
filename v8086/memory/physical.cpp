#include "physical.h"

#include <stdlib.h>

PhyMemory PhyMemoryAllocate(u32 capacity)
{
  PhyMemory res;

  res.Memory = (u8*) malloc(capacity * sizeof(*res.Memory));
  res.mask = capacity - 1;

  return res;
}

void PhyMemoryFree(PhyMemory& mem)
{
  if(PhyMemoryIsValid(mem))
  {
    free(mem.Memory);
    mem.Memory = nullptr;
    mem.mask = 0;
  }
}
