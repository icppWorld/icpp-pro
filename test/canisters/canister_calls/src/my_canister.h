#pragma once

#include "wasm_symbol.h"

// The same wasm is deployed three times, as canister_a, canister_b and
// canister_c, so the tests can make calls between canisters. Every method is available on
// both: the "callee" methods are called by the "caller" methods.

// Callee methods
void greet() WASM_SYMBOL_EXPORTED("canister_update greet");
void reject_me() WASM_SYMBOL_EXPORTED("canister_update reject_me");
void trap_me() WASM_SYMBOL_EXPORTED("canister_update trap_me");
void accept_half() WASM_SYMBOL_EXPORTED("canister_update accept_half");
void slow() WASM_SYMBOL_EXPORTED("canister_update slow");
void hold() WASM_SYMBOL_EXPORTED("canister_update hold");
void release() WASM_SYMBOL_EXPORTED("canister_update release");

// Caller methods
void call_greet() WASM_SYMBOL_EXPORTED("canister_update call_greet");
void call_reject() WASM_SYMBOL_EXPORTED("canister_update call_reject");
void call_trap() WASM_SYMBOL_EXPORTED("canister_update call_trap");
void call_then_trap() WASM_SYMBOL_EXPORTED("canister_update call_then_trap");
void call_with_cycles()
    WASM_SYMBOL_EXPORTED("canister_update call_with_cycles");
void call_bounded() WASM_SYMBOL_EXPORTED("canister_update call_bounded");
void call_sequence() WASM_SYMBOL_EXPORTED("canister_update call_sequence");
void call_chain() WASM_SYMBOL_EXPORTED("canister_update call_chain");
void call_oneway() WASM_SYMBOL_EXPORTED("canister_update call_oneway");
void yield_loop() WASM_SYMBOL_EXPORTED("canister_update yield_loop");
void budget_probe() WASM_SYMBOL_EXPORTED("canister_update budget_probe");
void start_held_call() WASM_SYMBOL_EXPORTED("canister_update start_held_call");
void compute() WASM_SYMBOL_EXPORTED("canister_update compute");
void compute_no_yield()
    WASM_SYMBOL_EXPORTED("canister_update compute_no_yield");
void get_job() WASM_SYMBOL_EXPORTED("canister_query get_job");
void get_run_log() WASM_SYMBOL_EXPORTED("canister_query get_run_log");
void clear_run_log() WASM_SYMBOL_EXPORTED("canister_update clear_run_log");
void get_state() WASM_SYMBOL_EXPORTED("canister_query get_state");
