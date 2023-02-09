#pragma once

#include "wasm_symbol.h"

void test_ic_api() WASM_SYMBOL_EXPORTED("canister_query test_ic_api");
void test_candid() WASM_SYMBOL_EXPORTED("canister_query test_candid");
void test_vendors() WASM_SYMBOL_EXPORTED("canister_query test_vendors");

void roundtrip_bool() WASM_SYMBOL_EXPORTED("canister_query roundtrip_bool");
void roundtrip_record() WASM_SYMBOL_EXPORTED("canister_query canister_record");

void canister_sends_record()
    WASM_SYMBOL_EXPORTED("canister_query canister_sends_record");

void canister_sends_int_as_int()
    WASM_SYMBOL_EXPORTED("canister_query canister_sends_int_as_int");
void canister_sends_double_as_float64()
    WASM_SYMBOL_EXPORTED("canister_query canister_sends_double_as_float64");
void canister_sends_char_as_text()
    WASM_SYMBOL_EXPORTED("canister_query canister_sends_char_as_text");
void canister_sends_string_as_text()
    WASM_SYMBOL_EXPORTED("canister_query canister_sends_string_as_text");
void canister_sends_json_as_text()
    WASM_SYMBOL_EXPORTED("canister_query canister_sends_json_as_text");

void roundtrip_text_to_json_to_text()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_text_to_json_to_text");
void roundtrip_text_to_json_to_text_long_message() WASM_SYMBOL_EXPORTED(
    "canister_query roundtrip_text_to_json_to_text_long_message");
