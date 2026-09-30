#pragma once

#include "wasm_symbol.h"

// The canister lifecycle hooks. Each one constructs an IC_API for its own
// entry point and records that it ran, so a test can prove the hooks ran on
// a real replica without trapping.
void canister_init() WASM_SYMBOL_EXPORTED("canister_init");
void canister_pre_upgrade() WASM_SYMBOL_EXPORTED("canister_pre_upgrade");
void canister_post_upgrade() WASM_SYMBOL_EXPORTED("canister_post_upgrade");

void get_upgrade_record()
    WASM_SYMBOL_EXPORTED("canister_query get_upgrade_record");
