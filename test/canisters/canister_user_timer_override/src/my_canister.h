#pragma once

#include <cstdint>

#include "wasm_symbol.h"

// Counters and ids exposed for the native test driver to inspect directly.
// g_user_dispatcher_calls increments inside the user's strong override of
// canister_global_timer (defined in my_canister.cpp). g_icpp_callback_calls
// increments inside a recurring timer registered through the icpp-pro
// IcTimers registry — if the icpp-pro built-in dispatcher had run, the
// callback would fire and this counter would advance; if the user's strong
// override won at link time, this counter stays at 0.
extern uint64_t g_user_dispatcher_calls;
extern uint64_t g_icpp_callback_calls;

void canister_init() WASM_SYMBOL_EXPORTED("canister_init");

// Strong user-supplied canister_global_timer. The icpp-pro side defines
// its own version with __attribute__((weak)), so wasm-ld picks this one.
void canister_global_timer() WASM_SYMBOL_EXPORTED("canister_global_timer");

void start_user_timer()
    WASM_SYMBOL_EXPORTED("canister_update start_user_timer");
void stop_user_timer() WASM_SYMBOL_EXPORTED("canister_update stop_user_timer");

void get_user_dispatcher_calls()
    WASM_SYMBOL_EXPORTED("canister_query get_user_dispatcher_calls");
void get_icpp_callback_calls()
    WASM_SYMBOL_EXPORTED("canister_query get_icpp_callback_calls");
void get_icpp_timer_count()
    WASM_SYMBOL_EXPORTED("canister_query get_icpp_timer_count");
