// Macros for importing & exporting Wasm symbols
#pragma once

// See: https://lld.llvm.org/WebAssembly.html#imports
#define WASM_SYMBOL_IMPORTED(module, name)                                     \
  __attribute__((import_module(module))) __attribute__((import_name(name)));

// See: https://lld.llvm.org/WebAssembly.html#exports
#define WASM_SYMBOL_EXPORTED(name)                                             \
  asm(name) __attribute__((visibility("default")));
