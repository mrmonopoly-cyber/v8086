#pragma once

#include <iostream>
#include <array>

#include "types.hpp"
#include "registers.hpp"

/** For jumps above and below refer to the relationship of two UNSIGNED VALUES */
/** For jumps greater and less refer to the relationship of two SIGNED VALUES */
enum Opcode : u8
{
#define ORG(Mnemonic, ...) Op_##Mnemonic, 
#define VAR(Mnemonic, ...)
#include "../v8086_instruction_table.inl"
  INVALID_OP
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
