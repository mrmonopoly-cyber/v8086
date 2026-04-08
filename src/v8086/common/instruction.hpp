#pragma once

#include "types.hpp"
#include <array>
#include <iostream>

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
enum Opcode : u8
{
#define X(OP, asm_name, descr) OP,
  OpCodes
  SHARED, //check second byte
  INVALID_OP
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
#define X(Reg, OPS, DESCR) Reg,
  Regs
#undef X
  INVALID_REG
};

struct Addr
{
  enum Length{
    _8_bit,
    _16_bit,

    ___Length_len
  };

  Addr(Length len, ::std::size_t raw) noexcept : len(len), raw(raw) {}

  Length len;
  ::std::size_t raw;
};

class Instruction{
  public:
    enum OpInputType : char
    {
      In_Reg_Reg,
      In_Reg_Mem,
      In_Mem_Reg,

      In_no_arg,
    };

    struct InstructionStr{
      ::std::array<char, 64> data;

      inline friend ::std::ostream& operator<<(::std::ostream& out, const InstructionStr& i)
      {
        out << &i.data[0];
        return out;
      }

      inline const char* cstr() const noexcept{
        return data.data();
      };
    };

    Instruction() noexcept : op(INVALID_OP), in_type(In_no_arg), data() {};

    Instruction(Opcode op, Reg r1, Reg r2) noexcept :
      op(op), in_type(In_Reg_Reg), data(r1, r2) {}

    Instruction(Opcode op, Reg r1, Addr addr) noexcept :
      op(op), in_type(In_Reg_Mem), data(r1, addr) {}

    Instruction(Opcode op, Addr addr, Reg r) noexcept :
      op(op), in_type(In_Mem_Reg), data(r, addr) {}

    InstructionStr to_string(void) const noexcept;

  private:
    Opcode op; 
    OpInputType in_type;
    union RegMemData{
      struct Reg_Reg{
        Reg fst;
        Reg snd;
      }reg_reg;
      struct Reg_Mem{
        Reg r;
        Addr a;
      }reg_mem;

      struct No_Arg{
      }no_arg;

      inline RegMemData(Reg r1, Reg r2) noexcept : reg_reg(Reg_Reg{r1, r2}) {}
      inline RegMemData(Reg r, Addr addr) noexcept : reg_mem(Reg_Mem{r, addr}) {}
      inline RegMemData() noexcept : no_arg() {}
    }data;
};
