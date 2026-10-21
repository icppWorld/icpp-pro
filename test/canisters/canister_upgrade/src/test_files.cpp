// Files in the canister's file system, which lives in stable memory.
//
// They guard two things:
// - files written by a canister built with a previous icpp-pro release read
//   back intact after an upgrade, also when the file system changed (7.0.0rc3
//   upgraded it from stable-fs 0.5.1 to 0.13.0);
// - a file overwritten in place by a smaller one reads like a new file of
//   that size. stable-fs 0.5.1 kept the chunks of the larger file, and a read
//   that asked for more than the file held walked over all of them, which on
//   a large file exceeded the IC's limit on stable memory accessed per
//   message.

#include "test_files.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "ic_api.h"

namespace {
// The byte at offset i of a file written by write_file
uint8_t pattern(uint64_t i) { return static_cast<uint8_t>(i * 31 + 7); }

uint64_t instructions() {
  return IC_API::performance_counter(
      IC_API::PerformanceCounterType::Instructions);
}
} // namespace

// Writes a file of mb MiB, in writes of 2 MiB, truncating what was there:
// '("path", mb)' -> '(bytes : nat64)'
void write_file() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  std::string path;
  uint32_t mb{0};
  CandidArgs args;
  args.append(CandidTypeText{&path});
  args.append(CandidTypeNat32{&mb});
  ic_api.from_wire(args);

  constexpr uint64_t WRITE_SIZE = 2 * 1024 * 1024;
  uint64_t size = uint64_t(mb) * 1024 * 1024;
  std::vector<char> buffer(WRITE_SIZE);
  std::ofstream file(path, std::ios::binary | std::ios::trunc);
  for (uint64_t offset = 0; offset < size; offset += WRITE_SIZE) {
    uint64_t n = std::min(WRITE_SIZE, size - offset);
    for (uint64_t i = 0; i < n; ++i)
      buffer[i] = static_cast<char>(pattern(offset + i));
    file.write(buffer.data(), static_cast<std::streamsize>(n));
  }
  file.close();
  if (!file) IC_API::trap("write_file: failed to write " + path);
  ic_api.to_wire(CandidTypeNat64{size});
}

// Reads a whole file with one read that asks for 1 MiB more than the file
// holds, and checks every byte:
// '("path")' -> '("bytes: 1048576; ok: true; instructions: 123")'
void read_file() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::string path;
  ic_api.from_wire(CandidTypeText{&path});

  uint64_t start = instructions();
  std::FILE *file = std::fopen(path.c_str(), "rb");
  if (!file) IC_API::trap("read_file: failed to open " + path);
  std::fseek(file, 0, SEEK_END);
  uint64_t size = static_cast<uint64_t>(std::ftell(file));
  std::fseek(file, 0, SEEK_SET);
  std::vector<char> buffer(size + 1024 * 1024);
  uint64_t bytes = std::fread(buffer.data(), 1, buffer.size(), file);
  std::fclose(file);
  uint64_t used = instructions() - start;

  bool ok = bytes == size;
  for (uint64_t i = 0; ok && i < bytes; ++i)
    ok = static_cast<uint8_t>(buffer[i]) == pattern(i);
  ic_api.to_wire(CandidTypeText{"bytes: " + std::to_string(bytes) +
                                "; ok: " + (ok ? "true" : "false") +
                                "; instructions: " + std::to_string(used)});
}
