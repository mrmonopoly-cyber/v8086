#include "instruction.hpp"
#include "utils.hpp"
#include <string_view>

static inline int write_reg(char* data, std::size_t len, Reg r)
{
  std::string_view sw{};
  switch (r)
  {
#define X(reg, regs, descr) case reg: sw = regs; break;
    Regs
#undef X
  }

  return sw.copy(data, len);
}

Instruction::InstructionStr Instruction::to_string(void) const noexcept
{
  InstructionStr res{};
  std::string_view ops{};
  std::size_t cursor{};

  switch (this->op)
  {
#define X(OP, asm_name, descr) case OP:ops = asm_name;break;
    OpCodes
#undef X
  }

  cursor += ops.copy(res.data.data(), res.data.size());
  res.data[cursor++] = ',';

  switch (this->in_type)
  {
    case In_Reg_Reg:
      cursor += write_reg(&res.data.at(cursor), res.data.size() - cursor, this->reg_reg.fst);
      res.data[cursor++] = ',';
      cursor += write_reg(&res.data.at(cursor), res.data.size() - cursor, this->reg_reg.snd);
      break;
    case In_Reg_Mem8:
      TODO("memory arg");
      res.data[cursor++] = ',';
      cursor += write_reg(&res.data.at(cursor), res.data.size() - cursor, this->reg_reg.fst);
      break;
    case In_Mem8_Reg:
      cursor += write_reg(&res.data.at(cursor), res.data.size() - cursor, this->reg_reg.fst);
      res.data[cursor++] = ',';
      TODO("memory arg");
      break;
    case In_Reg_Mem16:
      TODO("memory arg");
      res.data[cursor++] = ',';
      cursor += write_reg(&res.data.at(cursor), res.data.size() - cursor, this->reg_reg.fst);
      break;
    case In_Mem16_Reg:
      cursor += write_reg(&res.data.at(cursor), res.data.size() - cursor, this->reg_reg.fst);
      res.data[cursor++] = ',';
      TODO("memory arg");
      break;
  }

  res.data[cursor++] = ',';

  res.data[cursor] = '\0';
  return res;
}
