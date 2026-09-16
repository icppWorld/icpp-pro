// Functions that can be imported from the ic0 wasm runtime
// This header is imported by both C & C++ code
// See: https://isocpp.org/wiki/faq/mixing-c-and-cpp#include-c-hdrs-personal

#ifdef __cplusplus

// only include this in C++ files
// Ensure the WASI polyfill library is initialized first
#include "ic_wasi_polyfill.h"

extern "C" {
#endif

#pragma once

#include <stdint.h>

#include "wasm_symbol.h"

// See:
// https://internetcomputer.org/docs/current/references/ic-interface-spec#system-api-imports
uint32_t ic0_msg_arg_data_size()
    WASM_SYMBOL_IMPORTED("ic0", "msg_arg_data_size");

void ic0_msg_arg_data_copy(uint32_t dst, uint32_t off, uint32_t size)
    WASM_SYMBOL_IMPORTED("ic0", "msg_arg_data_copy");

uint32_t ic0_msg_caller_size() WASM_SYMBOL_IMPORTED("ic0", "msg_caller_size");

void ic0_msg_caller_copy(uint32_t dst, uint32_t off, uint32_t size)
    WASM_SYMBOL_IMPORTED("ic0", "msg_caller_copy");

uint32_t ic0_msg_reject_code() WASM_SYMBOL_IMPORTED("ic0", "msg_reject_code");

uint32_t ic0_msg_reject_msg_size()
    WASM_SYMBOL_IMPORTED("ic0", "msg_reject_msg_size");

void ic0_msg_reject_msg_copy(uint32_t dst, uint32_t off, uint32_t size)
    WASM_SYMBOL_IMPORTED("ic0", "msg_reject_msg_copy");

void ic0_msg_reply_data_append(uint32_t src, uint32_t size)
    WASM_SYMBOL_IMPORTED("ic0", "msg_reply_data_append");

void ic0_msg_reply() WASM_SYMBOL_IMPORTED("ic0", "msg_reply");

void ic0_msg_reject(uint32_t src, uint32_t size)
    WASM_SYMBOL_IMPORTED("ic0", "msg_reject");

// Ingress message inspection - only available in canister_inspect_message
uint32_t ic0_msg_method_name_size()
    WASM_SYMBOL_IMPORTED("ic0", "msg_method_name_size");

void ic0_msg_method_name_copy(uint32_t dst, uint32_t off, uint32_t size)
    WASM_SYMBOL_IMPORTED("ic0", "msg_method_name_copy");

void ic0_accept_message() WASM_SYMBOL_IMPORTED("ic0", "accept_message");

uint32_t ic0_canister_self_size()
    WASM_SYMBOL_IMPORTED("ic0", "canister_self_size");

void ic0_canister_self_copy(uint32_t dst, uint32_t off, uint32_t size)
    WASM_SYMBOL_IMPORTED("ic0", "canister_self_copy");

void ic0_canister_cycle_balance128(uint32_t dst)
    WASM_SYMBOL_IMPORTED("ic0", "canister_cycle_balance128");

void ic0_canister_liquid_cycle_balance128(uint32_t dst)
    WASM_SYMBOL_IMPORTED("ic0", "canister_liquid_cycle_balance128");

// 128-bit cycles convention: amounts passed IN are a (high, low) uint64_t
// pair; amounts written OUT are 16 little-endian bytes at dst (low uint64_t
// at offset 0, high at offset 8) - i.e. a little-endian __uint128_t.
void ic0_msg_cycles_available128(uint32_t dst)
    WASM_SYMBOL_IMPORTED("ic0", "msg_cycles_available128");

void ic0_msg_cycles_refunded128(uint32_t dst)
    WASM_SYMBOL_IMPORTED("ic0", "msg_cycles_refunded128");

void ic0_msg_cycles_accept128(uint64_t max_amount_high, uint64_t max_amount_low,
                              uint32_t dst)
    WASM_SYMBOL_IMPORTED("ic0", "msg_cycles_accept128");

void ic0_cycles_burn128(uint64_t amount_high, uint64_t amount_low, uint32_t dst)
    WASM_SYMBOL_IMPORTED("ic0", "cycles_burn128");

void ic0_call_new(uint32_t callee_src, uint32_t callee_size, uint32_t name_src,
                  uint32_t name_size, uint32_t reply_fun, uint32_t reply_env,
                  uint32_t reject_fun, uint32_t reject_env)
    WASM_SYMBOL_IMPORTED("ic0", "call_new");

void ic0_call_on_cleanup(uint32_t fun, uint32_t env)
    WASM_SYMBOL_IMPORTED("ic0", "call_on_cleanup");

void ic0_call_data_append(uint32_t src, uint32_t size)
    WASM_SYMBOL_IMPORTED("ic0", "call_data_append");

// Only valid between call_new and call_perform. No IC_API wrapper yet - that
// belongs to the future async inter-canister call API.
void ic0_call_cycles_add128(uint64_t amount_high, uint64_t amount_low)
    WASM_SYMBOL_IMPORTED("ic0", "call_cycles_add128");

uint32_t ic0_call_perform() WASM_SYMBOL_IMPORTED("ic0", "call_perform");

uint32_t ic0_stable_size() WASM_SYMBOL_IMPORTED("ic0", "stable_size");

uint32_t ic0_stable_grow(uint32_t new_pages)
    WASM_SYMBOL_IMPORTED("ic0", "stable_grow");

void ic0_stable_write(uint32_t off, uint32_t src, uint32_t size)
    WASM_SYMBOL_IMPORTED("ic0", "stable_write");

void ic0_stable_read(uint32_t dst, uint32_t off, uint32_t size)
    WASM_SYMBOL_IMPORTED("ic0", "stable_read");

// Certified data. The value set may be at most 32 bytes; the data
// certificate is only present in a (composite) query call.
void ic0_certified_data_set(uint32_t src, uint32_t size)
    WASM_SYMBOL_IMPORTED("ic0", "certified_data_set");

uint32_t ic0_data_certificate_present()
    WASM_SYMBOL_IMPORTED("ic0", "data_certificate_present");

uint32_t ic0_data_certificate_size()
    WASM_SYMBOL_IMPORTED("ic0", "data_certificate_size");

void ic0_data_certificate_copy(uint32_t dst, uint32_t off, uint32_t size)
    WASM_SYMBOL_IMPORTED("ic0", "data_certificate_copy");

uint64_t ic0_time() WASM_SYMBOL_IMPORTED("ic0", "time");

uint64_t ic0_global_timer_set(uint64_t timestamp_ns)
    WASM_SYMBOL_IMPORTED("ic0", "global_timer_set");

uint32_t ic0_is_controller(uint32_t src, uint32_t size)
    WASM_SYMBOL_IMPORTED("ic0", "is_controller");

void ic0_debug_print(uint32_t src, uint32_t size)
    WASM_SYMBOL_IMPORTED("ic0", "debug_print");

[[noreturn]] void ic0_trap(uint32_t src, uint32_t size)
    WASM_SYMBOL_IMPORTED("ic0", "trap");

#ifdef __cplusplus
}
#endif