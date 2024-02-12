#pragma once

#include "wasm_symbol.h"

void query_greet() WASM_SYMBOL_EXPORTED("canister_query query_greet");
void query_greet32() WASM_SYMBOL_EXPORTED("canister_query query_greet32");
void query_greet64() WASM_SYMBOL_EXPORTED("canister_query query_greet64");

void update_greet() WASM_SYMBOL_EXPORTED("canister_update update_greet");
void update_greet32() WASM_SYMBOL_EXPORTED("canister_update update_greet32");
void update_greet64() WASM_SYMBOL_EXPORTED("canister_update update_greet64");