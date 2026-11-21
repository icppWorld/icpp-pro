#include "ic_wasi_polyfill.h"

#include <iostream>

void raw_init(char *p, std::size_t len) {
  std::cout << "ic0mock raw_init" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
}

// Native builds use the host's random_get, which needs no seed
void raw_init_seed(const unsigned char *seed, std::size_t len) {}

// Native builds use the host's file system, which has no chunks. As on the IC
// once a file exists, the first valid size is kept, and any other is refused
int32_t raw_set_chunk_size(uint32_t size) {
  static uint32_t chunk_size{0};
  bool valid{false};
  for (uint32_t v : {4096u, 8192u, 16384u, 32768u, 65536u})
    valid = valid || size == v;
  if (!valid || (chunk_size != 0 && size != chunk_size)) return 28; // EINVAL
  chunk_size = size;
  return 0;
}
