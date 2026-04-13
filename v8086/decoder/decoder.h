#pragma once

#include <stdio.h>

#include <v8086_definitions.h>

struct EncodedInstruction{
  u8 data[4];
  u8 len;
};

struct Instruction{
};

void InstructionPrint(const Instruction& instr, FILE* out);

void EncodedInstructionPrintMnemonic(const EncodedInstruction& instr, FILE* out);

u32 InstructionDecode(const EncodedInstruction& instr, Instruction* const out);
