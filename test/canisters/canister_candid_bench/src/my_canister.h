#pragma once

#include "wasm_symbol.h"

void decode_smtp_request()
    WASM_SYMBOL_EXPORTED("canister_query decode_smtp_request");
void decode_mail_send() WASM_SYMBOL_EXPORTED("canister_query decode_mail_send");
void decode_blob() WASM_SYMBOL_EXPORTED("canister_query decode_blob");
void decode_text() WASM_SYMBOL_EXPORTED("canister_query decode_text");
void decode_vec_nat64() WASM_SYMBOL_EXPORTED("canister_query decode_vec_nat64");
void decode_vec_record10()
    WASM_SYMBOL_EXPORTED("canister_query decode_vec_record10");

void encode_mail_list() WASM_SYMBOL_EXPORTED("canister_query encode_mail_list");
void encode_mail_get() WASM_SYMBOL_EXPORTED("canister_query encode_mail_get");
void encode_mail_export_chunk()
    WASM_SYMBOL_EXPORTED("canister_query encode_mail_export_chunk");
void encode_blob() WASM_SYMBOL_EXPORTED("canister_query encode_blob");
void encode_text() WASM_SYMBOL_EXPORTED("canister_query encode_text");
void encode_vec_nat64() WASM_SYMBOL_EXPORTED("canister_query encode_vec_nat64");
void encode_vec_record10()
    WASM_SYMBOL_EXPORTED("canister_query encode_vec_record10");
void encode_record() WASM_SYMBOL_EXPORTED("canister_query encode_record");
void encode_small() WASM_SYMBOL_EXPORTED("canister_query encode_small");
