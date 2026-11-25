// Macros for importing & exporting Wasm symbols
#pragma once

// See: https://lld.llvm.org/WebAssembly.html#imports
#define WASM_SYMBOL_IMPORTED(module, name)                                     \
  __attribute__((import_module(module))) __attribute__((import_name(name)));

// See: https://lld.llvm.org/WebAssembly.html#exports
// The wasm exports only what is named this way: the IC refuses a module with
// more than 1100 exports, so the link no longer uses --export-dynamic.
// export_name is a wasm attribute; a native build only needs the asm label.
#if defined(__wasm__)
#define WASM_SYMBOL_EXPORTED(name)                                             \
  asm(name) __attribute__((visibility("default"), export_name(name)));
#else
#define WASM_SYMBOL_EXPORTED(name)                                             \
  asm(name) __attribute__((visibility("default")));
#endif
