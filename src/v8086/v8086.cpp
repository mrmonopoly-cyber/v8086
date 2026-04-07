#include "v8086.hpp"

#include <fstream>

int V8086::upload_program(::std::filesystem::path path) noexcept
{
  if(path.empty()) return 0;

  std::ifstream istream{path};
  char* ptr = reinterpret_cast<char*>(this->memory.data());
  istream.read(ptr, this->memory.size());
  this->program_length = path.u8string().length();

  return 0;
}

std::optional<Instruction> V8086::decode_next_instruction() const noexcept
{
  Instruction instr{};
  if(this->decode_index < this->program_length)
  {
    UNUSED(instr);
    TODO();
    return {};
  }
  return {};
}
