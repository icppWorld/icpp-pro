#pragma once

#include "wasm_symbol.h"

// The ingress-inspection hook: a user-declared strong symbol. icpp-pro
// deliberately ships no built-in canister_inspect_message export - exporting
// one turns on ingress filtering, which must be the canister author's
// explicit choice (see README-feature-guide.md, compatibility rule).
void canister_inspect_message()
    WASM_SYMBOL_EXPORTED("canister_inspect_message");

void allowed_greet() WASM_SYMBOL_EXPORTED("canister_update allowed_greet");
void blocked_greet() WASM_SYMBOL_EXPORTED("canister_update blocked_greet");
