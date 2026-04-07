#pragma once

#include <array>
#include <filesystem>
#include <optional>

#include "common.hpp"

class V8086
{
  public:
    V8086() : memory({})
    {
    }

    int load_program(::std::filesystem::path) noexcept;
    std::optional<Instruction> decode_next_instruction() const noexcept;

  private:
    std::array<u8, 1024 * 1024> memory;
};
