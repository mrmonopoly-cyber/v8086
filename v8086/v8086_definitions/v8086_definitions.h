#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ArraySize(Array) (sizeof(Array)/sizeof(Array[0]))

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

enum PowMem : u32{
  PowByte = 1,
  PowKilo = 10 * PowByte,
  PowMega = 2 * PowKilo,
  PowGiga = 3 * PowKilo,
};


#define TODO(...)\
  do{printf("%s.%d: TODO: %s\n", __FILE__, __LINE__, __VA_ARGS__""); exit(99);}while(0);
