#include "memory.h"

#include <stdio.h>
#include <stdlib.h>

static inline MemBlock* _free_list_at(FreeList& self, s32 index)
{
  if(index >=0 && index <  (s32)ArraySize(self.free_list_pool))
  {
    return &self.free_list_pool[index];
  }

  return nullptr;
}

static inline void _free_list_init(FreeList& self, u8* base, u32 capacity)
{
  MemBlock* root_block = nullptr;

  for(u32 i=1; i<ArraySize(self.free_index_queue); i++)
  {
    self.free_index_queue[i] = i;
    self.free_list_pool[i].next = -1;
  }

  self.next_free_index = 1;
  self.root = 0;

  root_block = &self.free_list_pool[self.root];
  root_block->base = base;
  root_block->capacity = capacity;
  root_block->in_use = 0;
  root_block->next = -1;
}

static inline s32 _free_list_next_free(FreeList& self)
{
  if(self.next_free_index >=0)
  {
    return self.free_index_queue[self.next_free_index--];
  }

  return -1;
}

static inline MemBlock* _free_list_find_first_fit(FreeList& self, u32 size)
{
  s32 index = self.root;
  MemBlock* res = &self.free_list_pool[index];

  while (res)
  {
    if((res->capacity - res->in_use) >= size)
    {
      break;
    }
    index = res->next;
    res = _free_list_at(self, index);
  }

  return res;
}

PhyMemory PhyMemoryAllocate()
{
  static u8 failed_allocation;
  PhyMemory res;
  const constexpr u32 mem_size = 1 << PowMega;

  res.Memory = (u8*) malloc(mem_size);
  if(res.Memory == nullptr)
  {
    res.Memory = &failed_allocation;
    res.capacity =0;
    res.mask =0;
    return res;
  }

  res.capacity = mem_size;
  res.mask = mem_size - 1;
  _free_list_init(res.free_list, res.Memory, res.capacity);

  return res;
}

LogicalSegment PhyMemorySaveSegment(PhyMemory& self, const u32 size)
{
  LogicalSegment res;
  s32 next_free_index = -1;
  MemBlock* next_free =nullptr;
  MemBlock* block = _free_list_find_first_fit(self.free_list, size);

  if(next_free_index == -1)
  {
    fprintf(stderr, "memory too fragmented\n");
    return res;
  }

  if(block)
  {
    next_free_index = _free_list_next_free(self.free_list);
    next_free = _free_list_at(self.free_list, next_free_index);
    next_free->capacity = block->capacity - block->in_use;
    next_free->next = block->next;
    next_free->base = block->base + block->in_use;

    block->next = next_free_index;
    block->capacity = block->in_use;
  }

  res.segment_base = next_free->base - self.Memory;
  res.segment_offset = 0;
  res.written = 0;

  return res;
}
