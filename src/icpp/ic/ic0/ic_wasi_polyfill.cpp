#include "ic_wasi_polyfill.h"

#include <cstring>

#include "ic0.h"

// The stable_fs_chunk_size of icpp.toml, or 0 to keep the size the file system
// has. icpp build-wasm generates its definition in the build folder; a wasm
// linked otherwise gets the weak 0 of icpp_stable_fs_chunk_size_default.c
extern "C" const uint32_t icpp_stable_fs_chunk_size;

void icpp_init_wasi_polyfill() {
  raw_init(nullptr, 0);
  // An upgrade to another size traps, so the canister keeps running its old
  // code on the file system it was created with
  if (icpp_stable_fs_chunk_size != 0 &&
      raw_set_chunk_size(icpp_stable_fs_chunk_size) != 0) {
    const char *msg =
        "ERROR: the file system's chunk size is not the stable_fs_chunk_size "
        "of icpp.toml, and it cannot change once a file exists. Build with the "
        "size the canister was installed with.";
    ic0_trap(reinterpret_cast<uintptr_t>(msg), std::strlen(msg));
  }
}
