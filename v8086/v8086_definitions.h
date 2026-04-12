#pragma once

#include <stdint.h>

#define ArraySize(Array) (sizeof(Array)/sizeof(Array[0]))

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint64_t u64;
typedef uint32_t u32;

typedef int8_t s8;
typedef int16_t s16;
typedef int64_t s64;
typedef int32_t s32;

enum PowMem : u32{
  PowKilo = 10,
  PowMega = 20,
  PowGiga = 30,
};
