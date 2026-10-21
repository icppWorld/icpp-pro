#pragma once

#include "wasm_symbol.h"

// Files in the canister's file system, which lives in stable memory: written
// before an upgrade, read back after it
void write_file() WASM_SYMBOL_EXPORTED("canister_update write_file");
void read_file() WASM_SYMBOL_EXPORTED("canister_query read_file");
