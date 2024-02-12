#pragma once

#include "wasm_symbol.h"

void greet() WASM_SYMBOL_EXPORTED("canister_query greet");
void greet32() WASM_SYMBOL_EXPORTED("canister_query greet32");
void greet64() WASM_SYMBOL_EXPORTED("canister_query greet64");