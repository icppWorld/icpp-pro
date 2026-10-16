#pragma once

#include "wasm_symbol.h"

// Ingest a state with update self-calls, then ask about it with a query, or
// with a composite query that calls queries of the canister itself.

// Ingest
void reset() WASM_SYMBOL_EXPORTED("canister_update reset");
void ingest() WASM_SYMBOL_EXPORTED("canister_update ingest");
void store() WASM_SYMBOL_EXPORTED("canister_update store");

// Ask
void ask() WASM_SYMBOL_EXPORTED("canister_query ask");
void ask_cq() WASM_SYMBOL_EXPORTED("canister_composite_query ask_cq");

// The instruction budget of a composite query and the queries it calls
void burn_q() WASM_SYMBOL_EXPORTED("canister_query burn_q");
void cq_burn() WASM_SYMBOL_EXPORTED("canister_composite_query cq_burn");
