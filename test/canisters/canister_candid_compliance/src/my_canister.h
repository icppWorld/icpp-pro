#pragma once

#include "wasm_symbol.h"

void suites() WASM_SYMBOL_EXPORTED("canister_query suites");
void run_suite() WASM_SYMBOL_EXPORTED("canister_query run_suite");
