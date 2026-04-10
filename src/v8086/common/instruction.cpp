#include "instruction.hpp"

#include <cassert>
#include <cstdio>
#include <string_view>

static inline int write_reg(char* data, ::std::size_t len, Reg r)
{
  std::string_view sw{};
  switch (r)
  {
#define X(reg, regs, descr) case reg: sw = regs; break;
  Regs
#undef X
    case INVALID_REG:
      assert(0 && "unreachable");
      break;
  }

  return sw.copy(data, len);
}

static inline int write_mem_addr(char* data, std::size_t len, Addr addr)
{
  ::std::size_t written = 0;

  switch (addr.len)
  {
    case Addr::_8_bit:
      written += snprintf(data + written , len - written, "byte");
      break;
    case Addr::_16_bit:
      written += snprintf(data + written, len - written, "word");
      break;
    case Addr::___Length_len:
      assert(false && "unreachable");
      break;
  }

  written += snprintf(data + written, len - written, " [%ld]", addr.raw);

  return written;
}

Instruction::InstructionStr Instruction::to_string(void) const noexcept
{
  InstructionStr res{};
  ::std::string_view ops{};
  ::std::size_t cursor{};
  std::size_t len = res.data.size();
  char* const raw_cursor = res.data.data();

  switch (this->op)
  {
#define X(OP, asm_name, descr) case OP:ops = asm_name;break;
  OpCodes
#undef X
      case INVALID_OP:
        ops = "invalid_op";
        break;
  case SHARED:
    assert(0 && "unreachable");
    break;
  }

  cursor += ops.copy(res.data.data(), res.data.size()); //opcode

  res.data[cursor++] = ' ';

  switch (this->in_type)
  {
    case In_Reg_Reg:
      cursor += write_reg(raw_cursor + cursor, len - cursor, this->data.reg_reg.fst); //reg
      res.data[cursor++] = ',';
      res.data[cursor++] = ' ';
      cursor += write_reg(raw_cursor + cursor, len - cursor, this->data.reg_reg.snd); //reg
      break;
    case In_Reg_Mem:
      cursor += write_mem_addr(raw_cursor + cursor, len - cursor, this->data.reg_mem.a); //addr
      res.data[cursor++] = ',';
      res.data[cursor++] = ' ';
      cursor += write_reg(raw_cursor + cursor, len - cursor, this->data.reg_mem.r); //reg
      break;
    case In_Mem_Reg:
      cursor += write_reg(raw_cursor + cursor, len - cursor, this->data.reg_mem.r); //reg
      res.data[cursor++] = ',';
      res.data[cursor++] = ' ';
      cursor += write_mem_addr(raw_cursor + cursor, len - cursor, this->data.reg_mem.a); //addr
      break;
    case In_no_arg:
      break;
    }

  res.data[cursor] = '\0';
  return res;
}
