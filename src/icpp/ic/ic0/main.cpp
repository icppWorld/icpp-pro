#include "ic_wasi_polyfill.h"

// You must have a main() function in your canister that calls 'raw_init'
// This is part of the ic_wasi_polyfill and allows wasi2ic
// to convert the wasi specific items to th IC.
//
int main() {
  raw_init(nullptr, 0);
  return 0;
}