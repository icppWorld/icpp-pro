#pragma once

#include "wasm_symbol.h"

void http_request() WASM_SYMBOL_EXPORTED("canister_query http_request");
void http_request_update()
    WASM_SYMBOL_EXPORTED("canister_update http_request_update");
