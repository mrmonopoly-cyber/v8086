#pragma once

#include <stdio.h>

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

    __Flag_count
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

static inline void flag_set(u8* flags, u16 values)
{
  *flags |= values;
}
static inline void flag_clear(u8 *flags, u16 values)
{
  *flags &= (~0) ^ values;
}

static inline u8 flag_get(const u8 flags, const Flags flag)
{
  return (flags & (1 << flag)) > 0;
}

static void inline flag_print(const u8 flags, FILE* out)
{
  static char flags_str[] = "C_P_A_ZSTIDO";

  for(u16 f= 0u; f<__Flag_count; f++)
  {
    if(flag_get(flags, (Flags)f))
    {
      fprintf(out, "%c", flags_str[f]);
    }
  }
}

static void inline flag_print_all(u8 flags, FILE* out)
{
  fprintf(out, "CF:%d, PF:%d, AF:%d, ZF:%d, SF:%d, TF:%d, IF:%d, DF:%d, OF:%d\n",
    flag_get(flags, CF), flag_get(flags, PF), flag_get(flags, AF), flag_get(flags, ZF),
    flag_get(flags, SF), flag_get(flags, TF), flag_get(flags, IF), flag_get(flags, DF),
    flag_get(flags, OF));
}
