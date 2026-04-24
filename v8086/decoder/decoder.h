#pragma once

#include <v8086_definitions.h>

u32 InstructionDecode(const u8* mem, const u32 mem_size, Instruction* out);
