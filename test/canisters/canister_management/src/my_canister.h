#pragma once

#include "wasm_symbol.h"

// The management canister helpers (ic_management.h), each making its call
void raw_rand() WASM_SYMBOL_EXPORTED("canister_update raw_rand");
void create_canister() WASM_SYMBOL_EXPORTED("canister_update create_canister");
void canister_status() WASM_SYMBOL_EXPORTED("canister_update canister_status");
void ecdsa() WASM_SYMBOL_EXPORTED("canister_update ecdsa");
void schnorr() WASM_SYMBOL_EXPORTED("canister_update schnorr");
void vetkd() WASM_SYMBOL_EXPORTED("canister_update vetkd");
void invalid_key() WASM_SYMBOL_EXPORTED("canister_update invalid_key");
void http_get() WASM_SYMBOL_EXPORTED("canister_update http_get");
void transform() WASM_SYMBOL_EXPORTED("canister_query transform");
void cost_http_v2() WASM_SYMBOL_EXPORTED("canister_query cost_http_v2");
void echo_vec_blob() WASM_SYMBOL_EXPORTED("canister_query echo_vec_blob");

// The lifecycle helpers (lifecycle.cpp)
void update_settings() WASM_SYMBOL_EXPORTED("canister_update update_settings");
void start_child() WASM_SYMBOL_EXPORTED("canister_update start_child");
void stop_child() WASM_SYMBOL_EXPORTED("canister_update stop_child");
void delete_child() WASM_SYMBOL_EXPORTED("canister_update delete_child");
void deposit_child() WASM_SYMBOL_EXPORTED("canister_update deposit_child");
void canister_status_full()
    WASM_SYMBOL_EXPORTED("canister_update canister_status_full");
void ecdsa_r1() WASM_SYMBOL_EXPORTED("canister_update ecdsa_r1");
