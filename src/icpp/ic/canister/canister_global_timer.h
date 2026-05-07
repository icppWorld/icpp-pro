#pragma once

#include "canister_base.h"
#include "wasm_symbol.h"
#include <string>

class CanisterGlobalTimer : public CanisterBase {
public:
  CanisterGlobalTimer();
  CanisterGlobalTimer(std::string calling_function);
  ~CanisterGlobalTimer();
};

// Built-in canister_global_timer dispatcher. Drains all due timers from
// the IcTimers registry and re-arms ic0.global_timer_set. Exported from
// the WASM canister; also linked into native builds so MockIC drivers
// can invoke it directly to simulate the IC firing the timer.
void canister_global_timer() WASM_SYMBOL_EXPORTED("canister_global_timer");