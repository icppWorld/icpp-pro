// The two IC system calls icpp-candid uses, for the fuzz targets.
//
// A trap becomes a C++ exception, which the targets catch: rejecting a
// message is the decoder working, not a finding. What the fuzzer looks for is
// a crash, a hang, a sanitizer report or runaway memory.

#include "ic0.h"

#include <cstdint>
#include <string>

void ic0_debug_print(uintptr_t, uint32_t) {}

[[noreturn]] void ic0_trap(uintptr_t src, uint32_t size) {
  throw FuzzTrap(std::string(reinterpret_cast<const char *>(src), size));
}
