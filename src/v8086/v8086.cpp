#include "v8086.hpp"

#include "utils.hpp"
#include <fstream>

int V8086::upload_program(::std::filesystem::path path) noexcept
{
  if(path.empty()) return 0;

  std::ifstream istream{path};
  char* ptr = reinterpret_cast<char*>(this->memory.data());
  istream.read(ptr, this->memory.size());

  return 0;
}

std::optional<Instruction> V8086::decode_next_instruction() const noexcept
{
  TODO("not yet implemented");
  return {};
}
