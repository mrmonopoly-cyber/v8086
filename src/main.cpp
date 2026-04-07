#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>

#include "v8086/utils.hpp"
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

  v8086.load_program(in_path);
  while((instr = v8086.decode_next_instruction()).has_value())
  {
    TODO("not yet implemented");
  }


  return 0;
}
