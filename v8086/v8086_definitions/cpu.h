#pragma once

#include "types.h"

enum Flags : u16
{
    CF = 0,
    PF = 2,
    AF = 4,
    ZF = 6,
    SF = 7,
    TF = 8,
    IF = 9,
    DF = 10,
    OF = 11,
};

struct SplitReg{
  u8 l;
  u8 h;
};

struct CPURegister{
  union{
    u16 _u16;
    SplitReg _half;
  };
};

enum FullRegs :size_t
{
  ax,
  bx,
  cx,
  dx,
  sp,
  bp,
  si,
  di,

  __reg_count
};

struct CPU{
  CPURegister regs[__reg_count];
  u16 ip;
  u8 flags;
};

static inline void CPU_flag_set(CPU* self, u16 values)
{
  self->flags |= values;
}
static inline void CPU_flag_clear(CPU* self, u16 values)
{
  self->flags &= (~0) ^ values;
}

static inline u8 CPU_flag_get(const CPU* const self, const Flags flag)
{
  return (self->flags & (1 << flag)) > 0;
}
