#pragma once

#include "wasm_symbol.h"

#include <memory>

// Self managed pointer to a wrapped uint64_t
class Counter {
public:
  uint64_t counter;
};

extern Counter *p_counter;

void canister_init() WASM_SYMBOL_EXPORTED("canister_init");