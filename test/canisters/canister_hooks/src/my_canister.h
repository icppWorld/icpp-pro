#pragma once

#include "wasm_symbol.h"

void greet() WASM_SYMBOL_EXPORTED("canister_query greet");
