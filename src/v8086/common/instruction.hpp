#pragma once

enum Opcode
{
  MOV,     /**Mov*/
  PUSH,    /**Push*/
  POP,     /**Pop*/
  XCHG,    /**exchange*/
  XLAT,    /**Translate*/
  IN,      /**In*/
  OUT,     /**Out*/

  LEA,     /**Load Effective Address*/
  LDS,     /**Load Pointer Using DS*/
  LES,     /**Load Pointer Using ES*/

  LAHF,    /**Lead Register AH from flags*/
  SAHF,    /**Lead Register AH into flags*/
  PUSHF,   /**PUSH Flags*/
  POPF,    /**Pop Flags*/

  ADD,     /**add*/
  ADC,     /**add with carry*/
  INC,     /**increment*/
  AAA,     /**ASCII adjust for addition*/
  DAA,     /**Decimal adjust for addition*/

  SUB,     /**sub*/
  SBB,     /**sub with carry*/
  DEC,     /**decrement*/
  NEG,     /**negate*/
  CMP,     /**compare*/
  AAS,     /**ASCII adjust for subtraction*/
  DAS,     /**Decimal adjust for subtraction*/

  MUL,     /**Multiply*/
  IMUL,    /**Integer Multiply*/
  AAM,     /**ASCII adjust for Multiply*/

  DIV,     /**Divide*/
  IDIV,    /**Integer Divide*/
  AAD,     /**ASCII adjust for Divide*/

  CBW,     /**Convert Byte To Word*/
  CWD,     /**Convert Word To Doubleword*/

  NOT,     /**Not*/
  AND,     /**And*/
  OR,      /**Or*/
  XOR,     /**Xor*/
  TEST,    /**Test*/

  SHL,     /**(SAL) Shift Logical Left*/
  SAL,     /** Shift Arithmetic Left*/
  SHR,     /** Shift Logical Right*/
  SAR,     /** Shift Arithmetic Right*/

  ROL,     /** Rotate Left*/
  ROR,     /** Rotate Right*/
  RCL,     /** Rotate through Carry Left*/
  RCR,     /** Rotate through Carry Right*/

  REP,     /**Repeat*/
  REPE,    /**Repeat While Equal*/
  REPZ,    /**Repeat While Zero*/
  REPNZ,   /**Repeat While Not Zero*/

  MOVS,    /**Move String*/
  MOVSB,   /**Move String Byte*/
  MOVSW,   /**Move String Word*/
  CMPS,    /**Compare String*/
  SCAS,    /**Scan String*/
  LODS,    /**source-string*/
  STOS,    /**Store string*/

  CALL,    /**Call*/
  RET,     /**Return*/
  JMP,     /**Jmp*/

  /** For jumps above and below refer to the relationship of two UNSIGNED VALUES */
  /** For jumps greater and less refer to the relationship of two SIGNED VALUES */
  JA,      /**Jump Above*/
  JNBE,    /**Jump Not Below*/
  JAE,     /**Jump Above or equal*/
  JNB,     /**Jump Not Below*/
  JB,      /**Jump Below*/
  JNAE,    /**Jump not Above nor equal*/
  JBE,     /**Jump Below or equal*/
  JNA,     /**Jump Not Above*/
  JC,      /**Jump Carry*/
  JE,      /**Jump Equal*/
  JZ,      /**Jump Zero*/

  JG,      /**Jump Greater*/
  JNLE,    /**Jump Not Less nor Equal*/
  JGE,     /**Jump Greater or equal*/
  JNL,     /**Jump Not Less*/
  JL,      /**Jump Less*/
  JNGE,    /**Jump not Greater nor equal*/
  JLE,     /**Jump Less or equal*/
  JNG,     /**Jump Not Greater*/
  JNC,     /**Jump Not Carry*/
  JNE,     /**Jump Not Equal*/
  JNZ,     /**Jump Not Zero*/

  JNO,     /**Jump Not overflow*/
  JNP,     /**Jump Not parity*/
  JPO,     /**Jump parity Odd*/
  JNS,     /**Jump Not Sign*/
  JO,      /**Jump overflow*/
  JP,      /**Jump parity*/
  JPE,     /**Jump parity equal*/
  JS,      /**Jump sign*/


  LOOP,    /**Loop*/
  LOOPE,   /**Loop While Equal*/
  LOOPZ,   /**Loop While Zero*/
  LOOPNE,  /**Loop While Not Equal*/
  LOOPNZ,  /**Loop While Not Zero*/
  JCX,     /**Jump if CX Zero*/

  INT,     /**Interrupt*/
  INTO,    /**Interrupt on overflow*/
  IRET,    /**Interrupt return*/

  CLC,     /**Clear Carry Flag*/
  CMC,     /**Complement Carry Flag*/
  STC,     /**Set Carry Flag*/
  CLD,     /**Clear Direction Flag*/
  STD,     /**Set Direction Flag*/
  CLI,     /**Clear Interrupt-enable Flag*/
  STI,     /**Set Interrupt-enable flag*/

  HLT,     /**Halt*/
  WAIT,    /**Wait*/

  ESC,     /**Escape*/
  LOCK,    /**Lock*/

  NOP,     /**Nop*/
};

enum Reg
{
  AX, AL, AH,  /**accumulator*/
  BX, BL, BH,  /**Base*/
  CX, CL, CH,  /**Count*/
  DX, DL, DH,  /**Data*/
  SP,          /**Stack Pointer*/
  BP,          /**Base Pointer*/
  SI,          /**Source Index*/
  DI,          /**Destination Index*/
  CS,          /**Code Segment*/
  DS,          /**Data Segment*/
  SS,          /**Stack Segment*/
  ES,          /**Extra Segment*/
  IP,          /**Instruction Pointer (can not access directly)*/
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
    char data[64];
  };

  InstructionStr to_string();
};
