#pragma once

#include "wasm_symbol.h"

void test_ic_api() WASM_SYMBOL_EXPORTED("canister_query test_ic_api");
void test_candid() WASM_SYMBOL_EXPORTED("canister_query test_candid");
void test_vendors() WASM_SYMBOL_EXPORTED("canister_query test_vendors");

void roundtrip_bool_true()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_bool_true");
void roundtrip_bool_false()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_bool_false");
void roundtrip_nat_101()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_nat_101");
void roundtrip_nat_1001()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_nat_1001");
void roundtrip_int_101()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_int_101");
void roundtrip_int_1001()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_int_1001");
void roundtrip_int_101_neg()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_int_101_neg");
void roundtrip_int_1001_neg()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_int_1001_neg");
void roundtrip_nat8_101()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_nat8_101");
void roundtrip_nat16_101()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_nat16_101");
void roundtrip_nat16_1001()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_nat16_1001");
void roundtrip_nat32_101()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_nat32_101");
void roundtrip_nat32_1_000_000_001()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_nat32_1_000_000_001");
void roundtrip_nat64_101()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_nat64_101");
void roundtrip_nat64_1_000_000_000_000_000_001() WASM_SYMBOL_EXPORTED(
    "canister_query roundtrip_nat64_1_000_000_000_000_000_001");
void roundtrip_int8_101()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_int8_101");
void roundtrip_int8_101_neg()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_int8_101_neg");
void roundtrip_int16_101()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_int16_101");
void roundtrip_int16_1001()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_int16_1001");
void roundtrip_int16_101_neg()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_int16_101_neg");
void roundtrip_int16_1001_neg()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_int16_1001_neg");
void roundtrip_int32_101()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_int32_101");
void roundtrip_int32_1_000_000_001()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_int32_1_000_000_001");
void roundtrip_int32_101_neg()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_int32_101_neg");
void roundtrip_int32_1_000_000_001_neg()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_int32_1_000_000_001_neg");
void roundtrip_int64_101()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_int64_101");
void roundtrip_int64_1_000_000_000_000_000_001() WASM_SYMBOL_EXPORTED(
    "canister_query roundtrip_int64_1_000_000_000_000_000_001");
void roundtrip_int64_101_neg()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_int64_101_neg");
void roundtrip_int64_1_000_000_000_000_000_001_neg() WASM_SYMBOL_EXPORTED(
    "canister_query roundtrip_int64_1_000_000_000_000_000_001_neg");
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
