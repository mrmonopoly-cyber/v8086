#include "v8086.hpp"

#include <fstream>

enum ArgType : u8
{
  ARG_NONE=0,

  ARG_REG8,
  ARG_REG16,
  ARG_REG8_MEM8,
  ARG_REG16_MEM16,
  ARG_IMMED8,
  ARG_IMMED16,
  ARG_SEG_OVERR_PREFIX,
  ARG_AX, ARG_AL, ARG_AH,
  ARG_BX, BRG_BL, BRG_BH,
  ARG_CX, BRG_CL, BRG_CH,
  ARG_DX, BRG_DL, BRG_DH,
  ARG_SP,
  ARG_BP,
  ARG_SI,
  ARG_DI,

  ARG_CS,
  ARG_DS,
  ARG_ES,
  ARG_SS,

  SHORT_LABEL,
};

#define ONE_OP(op) {(op)}
#define TWO_OP(op1, op2) {(op1), (op2)}
#define THREE_OP(op1, op2, op3) {(op1), (op2), (op3)}

static struct{
  Opcode op[3];
  ArgType fst;
  ArgType snd;
}MACHINE_INSTR_DECODING_TABLE[255] =
{
  {ONE_OP(ADD), ARG_REG8_MEM8, ARG_REG8},                         //0x00
  {ONE_OP(ADD), ARG_REG16_MEM16, ARG_REG16},                      //0x01
  {ONE_OP(ADD), ARG_REG8, ARG_REG8_MEM8},                         //0x02
  {ONE_OP(ADD), ARG_REG16, ARG_REG16_MEM16},                      //0x03
  {ONE_OP(ADD), ARG_AL, ARG_IMMED8},                              //0x04
  {ONE_OP(ADD), ARG_AX, ARG_IMMED16},                             //0x05

  {ONE_OP(PUSH), ARG_ES, ARG_NONE},                               //0x06
  {ONE_OP(POP), ARG_ES, ARG_NONE},                                //0x07

  {ONE_OP(OR), ARG_REG8_MEM8, ARG_REG8},                          //0x08
  {ONE_OP(OR), ARG_REG16_MEM16, ARG_REG16},                       //0x09
  {ONE_OP(OR), ARG_REG8, ARG_REG8_MEM8},                          //0x0A
  {ONE_OP(OR), ARG_REG16, ARG_REG16_MEM16},                       //0x0B
  {ONE_OP(OR), ARG_AL, ARG_IMMED8},                               //0x0C
  {ONE_OP(OR), ARG_AX, ARG_IMMED16},                              //0x0D

  {ONE_OP(PUSH), ARG_CS, ARG_NONE},                               //0x0E

  {ONE_OP(INVALID_OP), ARG_NONE, ARG_NONE},                       //0x0F

  {ONE_OP(ADC), ARG_REG8_MEM8, ARG_REG8},                         //0x10
  {ONE_OP(ADC), ARG_REG16_MEM16, ARG_REG16},                      //0x11
  {ONE_OP(ADC), ARG_REG8, ARG_REG8_MEM8},                         //0x12
  {ONE_OP(ADC), ARG_REG16, ARG_REG16_MEM16},                      //0x13
  {ONE_OP(ADC), ARG_AL, ARG_IMMED8},                              //0x14
  {ONE_OP(ADC), ARG_AX, ARG_IMMED16},                             //0x15

  {ONE_OP(PUSH), ARG_ES, ARG_NONE},                               //0x16
  {ONE_OP(POP), ARG_ES, ARG_NONE},                                //0x17

  {ONE_OP(SBB), ARG_REG8_MEM8, ARG_REG8},                         //0x18
  {ONE_OP(SBB), ARG_REG16_MEM16, ARG_REG16},                      //0x19
  {ONE_OP(SBB), ARG_REG8, ARG_REG8_MEM8},                         //0x1A
  {ONE_OP(SBB), ARG_REG16, ARG_REG16_MEM16},                      //0x1B
  {ONE_OP(SBB), ARG_AL, ARG_IMMED8},                              //0x1C
  {ONE_OP(SBB), ARG_AX, ARG_IMMED16},                             //0x1D

  {ONE_OP(PUSH), ARG_DS, ARG_NONE},                               //0x1E
  {ONE_OP(POP), ARG_DS, ARG_NONE},                                //0x1F

  {ONE_OP(AND), ARG_REG8_MEM8, ARG_REG8},                         //0x20
  {ONE_OP(AND), ARG_REG16_MEM16, ARG_REG16},                      //0x21
  {ONE_OP(AND), ARG_REG8, ARG_REG8_MEM8},                         //0x22
  {ONE_OP(AND), ARG_REG16, ARG_REG16_MEM16},                      //0x23
  {ONE_OP(AND), ARG_AL, ARG_IMMED8},                              //0x24
  {ONE_OP(AND), ARG_AX, ARG_IMMED16},                             //0x25

  {ONE_OP(INVALID_OP), ARG_ES, ARG_SEG_OVERR_PREFIX},             //0x26

  {ONE_OP(DAA), ARG_NONE, ARG_NONE},                              //0x27

  {ONE_OP(SUB), ARG_REG8_MEM8, ARG_REG8},                         //0x28
  {ONE_OP(SUB), ARG_REG16_MEM16, ARG_REG16},                      //0x29
  {ONE_OP(SUB), ARG_REG8, ARG_REG8_MEM8},                         //0x2A
  {ONE_OP(SUB), ARG_REG16, ARG_REG16_MEM16},                      //0x2B
  {ONE_OP(SUB), ARG_AL, ARG_IMMED8},                              //0x2C
  {ONE_OP(SUB), ARG_AX, ARG_IMMED16},                             //0x2D

  {ONE_OP(INVALID_OP), ARG_CS, ARG_SEG_OVERR_PREFIX},             //0x2E

  {ONE_OP(DAS), ARG_NONE, ARG_NONE},                              //0x2F

  {ONE_OP(XOR), ARG_REG8_MEM8, ARG_REG8},                         //0x30
  {ONE_OP(XOR), ARG_REG16_MEM16, ARG_REG16},                      //0x31
  {ONE_OP(XOR), ARG_REG8, ARG_REG8_MEM8},                         //0x32
  {ONE_OP(XOR), ARG_REG16, ARG_REG16_MEM16},                      //0x33
  {ONE_OP(XOR), ARG_AL, ARG_IMMED8},                              //0x34
  {ONE_OP(XOR), ARG_AX, ARG_IMMED16},                             //0x35

  {ONE_OP(INVALID_OP), ARG_SS, ARG_SEG_OVERR_PREFIX},             //0x36

  {ONE_OP(AAA), ARG_NONE, ARG_NONE},                              //0x37

  {ONE_OP(CMP), ARG_REG8_MEM8, ARG_REG8},                         //0x38
  {ONE_OP(CMP), ARG_REG16_MEM16, ARG_REG16},                      //0x39
  {ONE_OP(CMP), ARG_REG8, ARG_REG8_MEM8},                         //0x3A
  {ONE_OP(CMP), ARG_REG16, ARG_REG16_MEM16},                      //0x3B
  {ONE_OP(CMP), ARG_AL, ARG_IMMED8},                              //0x3C
  {ONE_OP(CMP), ARG_AX, ARG_IMMED16},                             //0x3D

  {ONE_OP(INVALID_OP), ARG_DS, ARG_SEG_OVERR_PREFIX},             //0x3E

  {ONE_OP(AAS), ARG_NONE, ARG_NONE},                              //0x3F

  {ONE_OP(INC), ARG_AX, ARG_NONE},                                //0x40
  {ONE_OP(INC), ARG_CX, ARG_NONE},                                //0x41
  {ONE_OP(INC), ARG_DX, ARG_NONE},                                //0x42
  {ONE_OP(INC), ARG_BX, ARG_NONE},                                //0x43
  {ONE_OP(INC), ARG_SP, ARG_NONE},                                //0x44
  {ONE_OP(INC), ARG_BP, ARG_NONE},                                //0x45
  {ONE_OP(INC), ARG_SI, ARG_NONE},                                //0x46
  {ONE_OP(INC), ARG_DI, ARG_NONE},                                //0x47

  {ONE_OP(DEC), ARG_AX, ARG_NONE},                                //0x48
  {ONE_OP(DEC), ARG_CX, ARG_NONE},                                //0x49
  {ONE_OP(DEC), ARG_DX, ARG_NONE},                                //0x4A
  {ONE_OP(DEC), ARG_BX, ARG_NONE},                                //0x4B
  {ONE_OP(DEC), ARG_SP, ARG_NONE},                                //0x4C
  {ONE_OP(DEC), ARG_BP, ARG_NONE},                                //0x4D
  {ONE_OP(DEC), ARG_SI, ARG_NONE},                                //0x4E
  {ONE_OP(DEC), ARG_DI, ARG_NONE},                                //0x4F

  {ONE_OP(PUSH), ARG_AX, ARG_NONE},                               //0x50
  {ONE_OP(PUSH), ARG_CX, ARG_NONE},                               //0x51
  {ONE_OP(PUSH), ARG_DX, ARG_NONE},                               //0x52
  {ONE_OP(PUSH), ARG_BX, ARG_NONE},                               //0x53
  {ONE_OP(PUSH), ARG_SP, ARG_NONE},                               //0x54
  {ONE_OP(PUSH), ARG_BP, ARG_NONE},                               //0x55
  {ONE_OP(PUSH), ARG_SI, ARG_NONE},                               //0x56
  {ONE_OP(PUSH), ARG_DI, ARG_NONE},                               //0x57

  {ONE_OP(POP), ARG_AX, ARG_NONE},                                //0x58
  {ONE_OP(POP), ARG_CX, ARG_NONE},                                //0x59
  {ONE_OP(POP), ARG_DX, ARG_NONE},                                //0x5A
  {ONE_OP(POP), ARG_BX, ARG_NONE},                                //0x5B
  {ONE_OP(POP), ARG_SP, ARG_NONE},                                //0x5C
  {ONE_OP(POP), ARG_BP, ARG_NONE},                                //0x5D
  {ONE_OP(POP), ARG_SI, ARG_NONE},                                //0x5E
  {ONE_OP(POP), ARG_DI, ARG_NONE},                                //0x5F

  {ONE_OP(INVALID_OP), ARG_NONE, ARG_NONE},                       //0x60
  {ONE_OP(INVALID_OP), ARG_NONE, ARG_NONE},                       //0x61
  {ONE_OP(INVALID_OP), ARG_NONE, ARG_NONE},                       //0x62
  {ONE_OP(INVALID_OP), ARG_NONE, ARG_NONE},                       //0x63
  {ONE_OP(INVALID_OP), ARG_NONE, ARG_NONE},                       //0x64
  {ONE_OP(INVALID_OP), ARG_NONE, ARG_NONE},                       //0x65
  {ONE_OP(INVALID_OP), ARG_NONE, ARG_NONE},                       //0x66
  {ONE_OP(INVALID_OP), ARG_NONE, ARG_NONE},                       //0x67
  {ONE_OP(INVALID_OP), ARG_NONE, ARG_NONE},                       //0x68
  {ONE_OP(INVALID_OP), ARG_NONE, ARG_NONE},                       //0x69
  {ONE_OP(INVALID_OP), ARG_NONE, ARG_NONE},                       //0x6A
  {ONE_OP(INVALID_OP), ARG_NONE, ARG_NONE},                       //0x6B
  {ONE_OP(INVALID_OP), ARG_NONE, ARG_NONE},                       //0x6C
  {ONE_OP(INVALID_OP), ARG_NONE, ARG_NONE},                       //0x6D
  {ONE_OP(INVALID_OP), ARG_NONE, ARG_NONE},                       //0x6E
  {ONE_OP(INVALID_OP), ARG_NONE, ARG_NONE},                       //0x6F

  {ONE_OP(JO),  SHORT_LABEL, ARG_NONE},                           //0x70
  {ONE_OP(JNO), SHORT_LABEL, ARG_NONE},                           //0x71
  {THREE_OP(JB, JNAE, JC), SHORT_LABEL, ARG_NONE},                //0x72
  {THREE_OP(JNB, JAE, JNC), SHORT_LABEL, ARG_NONE},               //0x73
  {TWO_OP(JE, JZ), SHORT_LABEL, ARG_NONE},                        //0x74
  {TWO_OP(JNE, JNZ), SHORT_LABEL, ARG_NONE},                      //0x75
  {TWO_OP(JBE, JNA), SHORT_LABEL, ARG_NONE},                      //0x76
  {TWO_OP(JNBE, JA), SHORT_LABEL, ARG_NONE},                      //0x77
  {ONE_OP(JS), SHORT_LABEL, ARG_NONE},                            //0x78
  {ONE_OP(JNS), SHORT_LABEL, ARG_NONE},                           //0x79
  {TWO_OP(JP, JPE), SHORT_LABEL, ARG_NONE},                       //0x7A
  {TWO_OP(JNP, JPO), SHORT_LABEL, ARG_NONE},                      //0x7B
  {TWO_OP(JL, JNGE), SHORT_LABEL, ARG_NONE},                      //0x7C
  {TWO_OP(JNL, JGE), SHORT_LABEL, ARG_NONE},                      //0x7D
  {TWO_OP(JLE, JNG), SHORT_LABEL, ARG_NONE},                      //0x7E
  {TWO_OP(JNLE, JG), SHORT_LABEL, ARG_NONE},                      //0x7F
};

int V8086::upload_program(::std::filesystem::path path) noexcept
{
  if(path.empty()) return 0;

  std::ifstream istream{path};
  char* ptr = reinterpret_cast<char*>(this->memory.data());
  istream.read(ptr, this->memory.size());
  this->program_length = path.string().length();

  return 0;
}

std::optional<Instruction> V8086::decode_next_instruction() const noexcept
{
  Instruction instr{};
  u32 i=0;
  u8 op;

  while(i < this->program_length)
  {
    op = this->memory[this->decode_index + i];

  }



  return {};
}
