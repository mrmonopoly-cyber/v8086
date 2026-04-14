#pragma once

#include <stdio.h>

#include <v8086_definitions.h>

struct Instruction{
};

void InstructionPrint(const Instruction& instr, FILE* out);

u32 InstructionDecode(const u8* mem, const u32 mem_size, Instruction* const out);
