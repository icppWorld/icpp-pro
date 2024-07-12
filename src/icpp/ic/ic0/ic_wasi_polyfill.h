#pragma once

// -------------------------------------------------------------------
// Make sure the ic_wasi_polyfill is part of the wasm file
// See: https://github.com/wasm-forge/demo2
#include <cstddef>

#define __IMPORT(module, name)                                                 \
  __attribute__((__import_module__(#module), __import_name__(#name)))
#define __EXPORT(name) __attribute__((__export_name__(#name)))

extern "C" void raw_init(char *p, std::size_t len) __IMPORT(polyfill, raw_init);

// -------------------------------------------------------------------