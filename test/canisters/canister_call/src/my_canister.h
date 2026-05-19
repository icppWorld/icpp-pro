// Test canister for ic_call.h — verifies the inter-canister call helper by
// performing a self-call: an update method `ping_self` schedules a call to
// the same canister's `echo(text)` method, then replies with the echoed
// payload via the registered on_reply callback.
//
// The test in test_apis.py installs this canister, invokes `ping_self("hi")`,
// and asserts the eventual reply is `"hi"`.

#pragma once

#include "ic_api.h"
#include "ic_call.h"

void canister_init_()      WASM_SYMBOL_EXPORTED("canister_init");

void echo()                WASM_SYMBOL_EXPORTED("canister_update echo");
void ping_self()           WASM_SYMBOL_EXPORTED("canister_update ping_self");

void get_last_echoed()     WASM_SYMBOL_EXPORTED("canister_query get_last_echoed");
void get_pending_count()   WASM_SYMBOL_EXPORTED("canister_query get_pending_count");
