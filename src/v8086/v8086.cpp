#include "v8086.hpp"

#include <fstream>

enum EncodedOpcode : u8
{
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
  EncodedOpcode op;

  while(i < this->program_length)
  {
    op = static_cast<EncodedOpcode>(this->memory[this->decode_index + i++]);
    switch (op)
    {
    
    }

  }



  return {};
}
