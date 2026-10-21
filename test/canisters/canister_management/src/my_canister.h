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
