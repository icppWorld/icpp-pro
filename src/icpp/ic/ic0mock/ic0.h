// Mocked ic0 functions to mimic the IC.
// Use in a native executable, for development, debug & testing.

// This header is imported by both C & C++ code
// See: https://isocpp.org/wiki/faq/mixing-c-and-cpp#include-c-hdrs-personal

#ifdef __cplusplus
extern "C" {
#endif

#pragma once

#include <cstdint>

// See:
// https://smartcontracts.org/docs/interface-spec/index.html#system-api-imports
uint32_t ic0_msg_arg_data_size();

void ic0_msg_arg_data_copy(uintptr_t dst, uint32_t off, uint32_t size);

uint32_t ic0_msg_caller_size();

void ic0_msg_caller_copy(uintptr_t dst, uint32_t off, uint32_t size);

uint32_t ic0_msg_reject_code();

uint32_t ic0_msg_reject_ic0_msg_size();

void ic0_msg_reject_ic0_msg_copy(uintptr_t dst, uint32_t off, uint32_t size);

void ic0_msg_reply_data_append(uintptr_t src, uint32_t size);

void ic0_msg_reply();

void ic0_msg_reject(uintptr_t src, uint32_t size);

uint64_t ic0_msg_funds_available(uintptr_t src, uint32_t size);

uint64_t ic0_msg_funds_refunded(uintptr_t src, uint32_t size);

void ic0_msg_funds_accept(uintptr_t src, uint32_t size, uint64_t amount);

uint32_t ic0_canister_self_size();

void ic0_canister_self_copy(uintptr_t dst, uint32_t off, uint32_t size);

uint64_t ic0_canister_balance(uintptr_t src, uint32_t size);

void ic0_call_new(uintptr_t callee_src, uint32_t callee_size,
                  uintptr_t name_src, uint32_t name_size, uintptr_t reply_fun,
                  uint32_t reply_env, uintptr_t reject_fun,
                  uint32_t reject_env);

void ic0_call_on_cleanup(uintptr_t fun, uint32_t env);

void ic0_call_data_append(uintptr_t src, uint32_t size);

void ic0_call_funds_add(uintptr_t src, uint32_t size, uint64_t amount);

uint32_t ic0_call_perform();
uint32_t ic0_stable_size();

uint32_t ic0_stable_grow(uint32_t new_pages);

void ic0_stable_write(uint32_t off, uintptr_t src, uint32_t size);

void ic0_stable_read(uintptr_t dst, uint32_t off, uint32_t size);

uint64_t ic0_time();

void ic0_debug_print(uintptr_t src, uint32_t size);

void ic0_trap(uintptr_t src, uint32_t size);

#ifdef __cplusplus
}
#endif