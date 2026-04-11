#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>

#include "v8086/v8086.hpp"

static inline void _help(void)
{
  printf("usage: v8086 <-h> [bin_file]\n");
  printf("\t -h \t\t print _help\n");
  printf("\t -o [file] \t output file name\n");
}

int main(int argc, char *argv[])
{
  ::std::string_view sw;
  V8086 v8086{};
  ::std::filesystem::path o_file_path;
  ::std::ofstream o_file;
  ::std::optional<Instruction> instr;
  ::std::filesystem::path in_path;
  int i=0;
  bool given_input = false;


  for(i=1; i<argc; i++)
  {
    sw = argv[i];
    if(!sw.compare("-h"))
    {
      _help();
      return 0;
    }
    else if(!sw.compare("-o"))
    {
      i++;
      if(i==argc)
      {
        _help();
        return 0;
      }
      o_file_path = argv[i];
    }
    else
    {
      given_input = true;
      in_path = argv[i];
    }
  }

  if(!given_input)
  {
    std::cout << "missing input program\n";
    _help();
    return 1;
  }

  // INFO: tests for print
  // Instruction hj{Op_mov, ax, bx};
  // Instruction hj1{Op_mov, ax, Addr{Addr::_8_bit, 69}};
  // Instruction hj2{Op_mov, bx, Addr{Addr::_16_bit, 42}};
  // Instruction hj3{Op_mov, Addr{Addr::_8_bit, 69}, cx};
  // Instruction hj4{Op_mov, Addr{Addr::_16_bit, 42}, dx};
  // Instruction hj5{};
  //
  // ::std::cout << hj.to_string() << std::endl;
  // ::std::cout << hj1.to_string() << std::endl;
  // ::std::cout << hj2.to_string() << std::endl;
  // ::std::cout << hj3.to_string() << std::endl;
  // ::std::cout << hj4.to_string() << std::endl;
  // ::std::cout << hj5.to_string() << std::endl;

  v8086.upload_program(in_path);
  while((instr = v8086.decode_next_instruction()).has_value())
  {
    std::cout << instr->to_string().data.data() << '\n';
    TODO("not yet implemented");
  }


  return 0;
}
