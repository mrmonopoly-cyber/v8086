#include "decoder.h"

#include <stdio.h>

#include <v8086_definitions.h>

static void _print_byte(u8 byte, FILE* out)
{
  fprintf(stderr, "0b");
  for(u8 i=0; i<8; i++)
  {
    if((byte>> (7 - i)) & 0x1)
    {
      fprintf(out, "1");
    }else
    {
      fprintf(out, "0");
    }
  }
  fprintf(stderr, " (%d)", byte);
}

void InstructionPrint(const Instruction& instr, FILE* out)
{
  TODO();
}

void EncodedInstructionPrintMnemonic(const EncodedInstruction& instr, FILE* out)
{
  const u8 mnemonic = instr.data[0];
  const u8 arg = instr.data[1];

  _print_byte(mnemonic, out);
  fprintf(out, ", ");
  _print_byte(arg, out);
}

u32 InstructionDecode(const EncodedInstruction& inst, Instruction* const out)
{
  return 0;
}
