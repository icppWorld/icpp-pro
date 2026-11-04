// The IC system calls icpp-candid uses, for the fuzz targets: see fuzz_ic0.cpp

#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

struct FuzzTrap : std::runtime_error {
  explicit FuzzTrap(const std::string &msg) : std::runtime_error(msg) {}
};

void ic0_debug_print(uintptr_t src, uint32_t size);
[[noreturn]] void ic0_trap(uintptr_t src, uint32_t size);
