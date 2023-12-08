#pragma once

#include <limits>
#include <math.h>
#include <variant>

#include "wasm_symbol.h"

void test_ic_api() WASM_SYMBOL_EXPORTED("canister_query test_ic_api");
void test_candid() WASM_SYMBOL_EXPORTED("canister_query test_candid");
void test_vendors() WASM_SYMBOL_EXPORTED("canister_query test_vendors");

void roundtrip_deprecated_ic_api_constructor() WASM_SYMBOL_EXPORTED(
    "canister_query roundtrip_deprecated_ic_api_constructor");
void roundtrip_no_arguments()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_no_arguments");
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
void roundtrip_text() WASM_SYMBOL_EXPORTED("canister_query roundtrip_text");
void roundtrip_text_to_json_to_text()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_text_to_json_to_text");
void roundtrip_text_to_json_to_text_long_message() WASM_SYMBOL_EXPORTED(
    "canister_query roundtrip_text_to_json_to_text_long_message");
void roundtrip_reserved()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_reserved");
void roundtrip_float32()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_float32");
void roundtrip_float32_neg()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_float32_neg");
void roundtrip_float64()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_float64");
void roundtrip_float64_neg()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_float64_neg");
void roundtrip_principal()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_principal");
void caller_is_anonymous()
    WASM_SYMBOL_EXPORTED("canister_query caller_is_anonymous");
void roundtrip_vec_nat16()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_vec_nat16");
void roundtrip_vec_record()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_vec_record");
void roundtrip_vec_all()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_vec_all");
void roundtrip_opt_nat()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_opt_nat");
void roundtrip_opt_nat_nat()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_opt_nat_nat");
void roundtrip_opt_nat_no_value()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_opt_nat_no_value");
void roundtrip_opt_nat16()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_opt_nat16");
void roundtrip_opt_nat16_no_value()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_opt_nat16_no_value");
void roundtrip_opt_record()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_opt_record");
void roundtrip_opt_record_no_value()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_opt_record_no_value");
void roundtrip_opt_variant_ok()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_opt_variant_ok");
void roundtrip_opt_variant_ok_no_value()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_opt_variant_ok_no_value");
void roundtrip_opt_variant_err()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_opt_variant_err");
void roundtrip_opt_variant_err_no_value()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_opt_variant_err_no_value");
void roundtrip_opt_record_variant()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_opt_record_variant");
void roundtrip_opt_record_variant_no_value() WASM_SYMBOL_EXPORTED(
    "canister_query roundtrip_opt_record_variant_no_value");
void roundtrip_opt_all()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_opt_all");
void roundtrip_record() WASM_SYMBOL_EXPORTED("canister_query roundtrip_record");
void roundtrip_record_opt()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_record_opt");
void roundtrip_record_opt_variant()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_record_opt_variant");
void roundtrip_opt_vec_nat16()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_opt_vec_nat16");
void roundtrip_opt_vec_record()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_opt_vec_record");
void roundtrip_record_record_record()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_record_record_record");
void roundtrip_record_vec_text()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_record_vec_text");
void roundtrip_record_vec_record()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_record_vec_record");
void roundtrip_variant_ok()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_variant_ok");
void roundtrip_variant_err()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_variant_err");
void roundtrip_variant()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_variant");
void roundtrip_variant_opt()
    WASM_SYMBOL_EXPORTED("canister_query roundtrip_variant_opt");

void trap_wrong_number_of_args()
    WASM_SYMBOL_EXPORTED("canister_query trap_wrong_number_of_args");
void trap_multiple_calls_from_wire()
    WASM_SYMBOL_EXPORTED("canister_query trap_multiple_calls_from_wire");
void trap_multiple_calls_to_wire()
    WASM_SYMBOL_EXPORTED("canister_query trap_multiple_calls_to_wire");

// from:https://stackoverflow.com/a/41405501/5480536
//implements relative method - do not use for comparing with zero
//use this most of the time, tolerance needs to be meaningful in your context
template <typename TReal>
static bool is_approximately_equal(
    TReal a, TReal b, TReal tolerance = std::numeric_limits<TReal>::epsilon()) {
  TReal diff = fabs(a - b);
  if (diff <= tolerance) return true;

  if (diff < fmax(fabs(a), fabs(b)) * tolerance) return true;

  return false;
}