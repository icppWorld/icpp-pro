#pragma once

#include <cstdint>

#include "wasm_symbol.h"

// Counters and ids exposed for the native test driver to inspect directly,
// without round-tripping through Candid.
extern uint64_t g_one_shot_count;
extern uint64_t g_recurring_count;

void canister_init() WASM_SYMBOL_EXPORTED("canister_init");

void start_one_shot() WASM_SYMBOL_EXPORTED("canister_update start_one_shot");
void start_recurring() WASM_SYMBOL_EXPORTED("canister_update start_recurring");
void stop_timer() WASM_SYMBOL_EXPORTED("canister_update stop_timer");
void reset_counts() WASM_SYMBOL_EXPORTED("canister_update reset_counts");

void get_one_shot_count()
    WASM_SYMBOL_EXPORTED("canister_query get_one_shot_count");
void get_recurring_count()
    WASM_SYMBOL_EXPORTED("canister_query get_recurring_count");
void get_timer_count() WASM_SYMBOL_EXPORTED("canister_query get_timer_count");
