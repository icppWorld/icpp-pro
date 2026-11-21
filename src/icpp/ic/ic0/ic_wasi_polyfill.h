#pragma once

// -------------------------------------------------------------------
// Make sure the ic_wasi_polyfill is part of the wasm file
// See: https://github.com/wasm-forge/demo2
#include <cstddef>
#include <cstdint>

#define __IMPORT(module, name)                                                 \
  __attribute__((__import_module__(#module), __import_name__(#name)))
#define __EXPORT(name) __attribute__((__export_name__(#name)))

extern "C" void raw_init(char *p, std::size_t len) __IMPORT(polyfill, raw_init);

// Reseeds the polyfill's random number generator, which serves WASI
// random_get (getentropy, std::random_device, arc4random). raw_init leaves it
// at a constant all-zero seed, so its output is predictable until a canister
// seeds it with IC_Management::seed_wasi_random().
extern "C" void raw_init_seed(const unsigned char *seed, std::size_t len)
    __IMPORT(polyfill, raw_init_seed);

// Sets the size of the file system's chunks, in bytes: 4096, 8192, 16384 (the
// default), 32768 or 65536. Returns 0, or 28 (EINVAL) for any other size, or for
// a different size once the file system holds a chunk. Setting the size it
// already has succeeds and changes nothing.
extern "C" int32_t raw_set_chunk_size(uint32_t size)
    __IMPORT(polyfill, raw_set_chunk_size);

// Calls raw_init, then sets the stable_fs_chunk_size of icpp.toml. It traps
// when the file system has another size: see ic_wasi_polyfill.cpp
void icpp_init_wasi_polyfill();

// -------------------------------------------------------------------
// Singleton class to ensure the WASI polyfill library is initialized only once.
class WasiPolyfill {
public:
  WasiPolyfill() {
    // A static local variable is initialized only once and
    // is thread-safe in C++11 and later
    static bool initialized = []() {
      icpp_init_wasi_polyfill();
      return true;
    }();
    (void)initialized; // To avoid unused variable warning
  }
};

inline WasiPolyfill
    __wasi_polyfill; // Inline variable, so there will be only one global instance to trigger initialization