#include "ic_wasi_polyfill.h"

#include <iostream>

void raw_init(char *p, std::size_t len) {
  std::cout << "ic0mock raw_init" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
}

// Native builds use the host's random_get, which needs no seed
void raw_init_seed(const unsigned char *seed, std::size_t len) {}
