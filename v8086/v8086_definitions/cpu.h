#pragma once

#include <stdio.h>

#include "types.h"

typedef u16 FlagsReg;

enum Flags : FlagsReg
{
    CF = 1 << 0,
    PF = 1 << 2,
    AF = 1 << 4,
    ZF = 1 << 6,
    SF = 1 << 7,
    TF = 1 << 8,
    IF = 1 << 9,
    DF = 1 << 10,
    OF = 1 << 11,

    FlagsAll = (FlagsReg) ~0,
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
  FlagsReg flags;
};

static inline void flag_set(FlagsReg* flags, FlagsReg values)
{
  *flags |= values;
}
static inline void flag_clear(FlagsReg *flags, FlagsReg values)
{
  *flags &= (~0) ^ values;
}

static inline u8 flag_get(const FlagsReg flags, const Flags flag)
{
  return (flags & flag) > 0;
}

static void inline flag_print(const FlagsReg flags, FILE* out)
{
  static char flags_str[] = "C_P_A_ZSTIDO";

  for(u16 f= 0u; f<ArraySize(flags_str); f++)
  {
    if(flag_get(flags, (Flags) (1<<f)))
    {
      fprintf(out, "%c", flags_str[f]);
    }
  }
}

static void inline flag_print_all(FlagsReg flags, FILE* out)
{
  fprintf(out, "CF:%d, PF:%d, AF:%d, ZF:%d, SF:%d, TF:%d, IF:%d, DF:%d, OF:%d\n",
    flag_get(flags, CF), flag_get(flags, PF), flag_get(flags, AF), flag_get(flags, ZF),
    flag_get(flags, SF), flag_get(flags, TF), flag_get(flags, IF), flag_get(flags, DF),
    flag_get(flags, OF));
}
