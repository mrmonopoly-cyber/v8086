#pragma once

#include "types.hpp"
#include <array>

#define OpCodes \
    X(MOV, "mov", "Mov")\
    X(PUSH, "push", "Push")\
    X(POP, "pop", "Pop")\
    X(XCHG, "xchg", "exchange")\
    X(XLAT, "xlat", "Translate")\
    X(IN, "in", "In")\
    X(OUT, "out", "Out")\
    X(LEA, "lea", "Load Effective Address")\
    X(LDS, "lds", "Load Pointer Using DS")\
    X(LES, "les", "Load Pointer Using ES")\
    X(LAHF, "lahf", "Lead Register AH from flags")\
    X(SAHF, "sahf", "Lead Register AH into flags")\
    X(PUSHF, "pushf", "PUSH Flags")\
    X(POPF, "popf", "Pop Flags")\
    X(ADD, "add", "add")\
    X(ADC, "adc", "add with carry")\
    X(INC, "inc", "increment")\
    X(AAA, "aaa", "ASCII adjust for addition")\
    X(DAA, "daa", "Decimal adjust for addition")\
    X(SUB, "sub", "sub")\
    X(SBB, "sub", "sub with carry")\
    X(DEC, "dec", "decrement")\
    X(NEG, "neg", "negate")\
    X(CMP, "cmp", "compare")\
    X(AAS, "aas", "ASCII adjust for subtraction")\
    X(DAS, "das", "Decimal adjust for subtraction")\
    X(MUL, "mul", "Multiply")\
    X(IMUL, "imul", "Integer Multiply")\
    X(AAM, "aam", "ASCII adjust for Multiply")\
    X(DIV, "div", "divide")\
    X(IDIV, "idiv", "Integer Divide")\
    X(AAD, "aad", "ASCII adjust for Divide")\
    X(CBW, "cbw", "Convert Byte To Word")\
    X(CWD, "cwd", "Convert Word To Doubleword")\
    X(NOT, "not", "not")\
    X(AND, "and", "and")\
    X(OR, "or", "or")\
    X(XOR, "xor", "xor")\
    X(TEST, "test", "test")\
    X(SHL, "shl", "Shift Logical Left")\
    X(SAL, "sal", "Shift Arithmetic Left")\
    X(SHR, "shr", "Shift Logical Right")\
    X(SAR, "sar", "Shift Arithmetic Right")\
    X(ROL, "rol", "Rotate Left")\
    X(ROR, "ror", "Rotate Right")\
    X(RCL, "rcl", "Rotate through Carry Left")\
    X(RCR, "rcr", "Rotate through Carry Right")\
    X(REP, "rep", "Repeat")\
    X(REPE, "repe", "Repeat While Equal")\
    X(REPZ, "repz", "Repeat While Zero")\
    X(REPNZ, "repnz", "Repeat While Not Zero")\
    X(MOVS, "movs", "Move String")\
    X(MOVSB, "movsb", "Move String Byte")\
    X(MOVSW, "movsw", "Move String Word")\
    X(CMPS, "cmps", "Compare String")\
    X(SCAS, "scas", "Scan String")\
    X(LODS, "lods", "source-string")\
    X(STOS, "stos", "Store string")\
    X(CALL, "call", "call")\
    X(RET, "ret", "return")\
    X(JMP, "jmp", "Jmp")\
    X(JA, "ja", "Jump Above")\
    X(JNBE, "jnbe", "Jump Not Below")\
    X(JAE, "JAE", "Jump Above or equal")\
    X(JNB, "JNB", "Jump Not Below")\
    X(JB, "jb", "Jump Below")\
    X(JNAE, "jnae", "Jump not Above nor equal")\
    X(JBE, "jbe", "Jump Below or equal")\
    X(JNA, "jna", "Jump Not Above")\
    X(JC, "jc", "Jump Carry")\
    X(JE, "je", "Jump Equal")\
    X(JZ, "jz", "Jump Zero")\
    X(JG, "jg", "Jump Greater")\
    X(JNLE, "jnle", "Jump Not Less nor Equal")\
    X(JGE, "jge", "Jump Greater or equal")\
    X(JNL, "jnl", "Jump Not Less")\
    X(JL, "jl", "Jump Less")\
    X(JNGE, "jnge", "Jump not Greater nor equal")\
    X(JLE, "jle", "Jump Less or equal")\
    X(JNG, "jng", "Jump Not Greater")\
    X(JNC, "jnc", "Jump Not Carry")\
    X(JNE, "jne", "Jump Not Equal")\
    X(JNZ, "jnz", "Jump Not Zero")\
    X(JNO, "jno", "Jump Not overflow")\
    X(JNP, "jnp", "Jump Not parity")\
    X(JPO, "jpo", "Jump parity Odd")\
    X(JNS, "jns", "Jump Not Sign")\
    X(JO, "jo", "Jump overflow")\
    X(JP, "jp", "Jump parity")\
    X(JPE, "jpe", "Jump parity equal")\
    X(JS, "js", "Jump sign")\
    X(LOOP, "loop", "Loop")\
    X(LOOPE, "loope", "Loop While Equal")\
    X(LOOPZ, "loopz", "Loop While Zero")\
    X(LOOPNE, "loopne", "Loop While Not Equal")\
    X(LOOPNZ, "loopnz", "Loop While Not Zero")\
    X(JCX, "jcx", "Jump if CX Zero")\
    X(INT, "int", "Interrupt")\
    X(INTO, "into", "Interrupt on overflow")\
    X(IRET, "iret", "Interrupt return")\
    X(CLC, "clc", "Clear Carry Flag")\
    X(CMC, "cmc", "Complement Carry Flag")\
    X(STC, "stc", "Set Carry Flag")\
    X(CLD, "cld", "Clear Direction Flag")\
    X(STD, "std", "Set Direction Flag")\
    X(CLI, "cli", "Clear Interrupt-enable Flag")\
    X(STI, "sti", "Set Interrupt-enable flag")\
    X(HLT, "hlt", "Halt")\
    X(WAIT, "wait", "Wait")\
    X(ESC, "esc", "Escape")\
    X(LOCK, "lock", "Lock")\
    X(NOP, "nop", "Nop")

/** For jumps above and below refer to the relationship of two UNSIGNED VALUES */
/** For jumps greater and less refer to the relationship of two SIGNED VALUES */
enum Opcode
{
#define X(OP, asm_name, descr) OP,
  OpCodes
#undef X
};

#define Regs \
  X(AX, "ax", "accumulator")\
  X(AH, "ah", "ah")\
  X(AL, "al", "al")\
  X(BX, "bx", "base")\
  X(BH, "bh", "bh")\
  X(BL, "bl", "bl")\
  X(CX, "cx", "count")\
  X(CH, "ch", "ch")\
  X(CL, "cl", "cl")\
  X(DX, "dx", "data")\
  X(DH, "dh", "dh")\
  X(DL, "dl", "dl")\
  X(SP, "sp", "stack pointer")\
  X(BP, "bp", "Base Pointer")\
  X(SI, "si", "Source Index")\
  X(DI, "di", "Destination Index")\
  X(CS, "cs", "Code Segment")\
  X(DS, "ds", "Data Segment")\
  X(SS, "ss", "Stack Segment")\
  X(ES, "es", "Extra Segment")\
  X(IP, "ip", "Instruction Pointer")\


enum Reg
{
#define X(OP, OPS, DESCR) OP,
  Regs
#undef X
};

typedef u16 Addr16;
typedef u16 Addr8;

enum OpInputType : char
{
  In_Reg_Reg,
  In_Reg_Mem8,
  In_Mem8_Reg,
  In_Reg_Mem16,
  In_Mem16_Reg,
};

struct Reg_Reg{
  Reg fst;
  Reg snd;
};

struct Reg_Mem8{
  Reg fst;
  Addr8 addr8;
};

struct Reg_Mem16{
  Reg fst;
  Addr16 addr16;
};

struct Mem8_Reg{
  Addr8 addr8;
  Reg fst;
};

struct Mem16_Reg{
  Addr16 addr16;
  Reg fst;
};

struct Instruction{
  Opcode op; 
  OpInputType in_type;
  union{
    Reg_Reg reg_reg;
    Reg_Mem8 reg_mem8;
    Mem8_Reg mem8_reg;
    Reg_Mem16 reg_mem16;
    Mem16_Reg mem16_reg;
  };

  struct InstructionStr{
    std::array<char, 64> data;
  };

  InstructionStr to_string(void) const noexcept;
};
