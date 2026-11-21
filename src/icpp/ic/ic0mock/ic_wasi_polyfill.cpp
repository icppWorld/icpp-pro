#include "ic_wasi_polyfill.h"

#include <iostream>

void raw_init(char *p, std::size_t len) {
  std::cout << "ic0mock raw_init" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
}

// Native builds use the host's random_get, which needs no seed
void raw_init_seed(const unsigned char *seed, std::size_t len) {}

// Native builds use the host's file system, which has no chunks: a valid size
// is accepted, any other is refused as on the IC
int32_t raw_set_chunk_size(uint32_t size) {
  for (uint32_t valid : {4096u, 8192u, 16384u, 32768u, 65536u})
    if (size == valid) return 0;
  return 28; // EINVAL
}
