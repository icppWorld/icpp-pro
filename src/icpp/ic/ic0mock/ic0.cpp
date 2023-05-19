// Mocked ic0 functions to mimic the IC.
// Use in a native executable, for development, debug & testing.

#include "ic0.h"
#include "pro.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "global.h"
#include "mock_ic.h"

// See:
// https://smartcontracts.org/docs/interface-spec/index.html#system-api-imports
uint32_t ic0_msg_arg_data_size() {
  Pro().exit_if_not_pro();
  return 0;
};

void ic0_msg_arg_data_copy(uintptr_t dst, uint32_t off, uint32_t size) {
  Pro().exit_if_not_pro();
};

uint32_t ic0_msg_caller_size() {
  Pro().exit_if_not_pro();
  return 0;
};

void ic0_msg_caller_copy(uintptr_t dst, uint32_t off, uint32_t size) {
  Pro().exit_if_not_pro();
};

uint32_t ic0_msg_reject_code() {
  Pro().exit_if_not_pro();
  return 0;
};

uint32_t ic0_msg_reject_msg_size() {
  Pro().exit_if_not_pro();
  return 0;
};

void ic0_msg_reject_msg_copy(uintptr_t dst, uint32_t off, uint32_t size) {
  Pro().exit_if_not_pro();
};

void ic0_msg_reply_data_append(uintptr_t src, uint32_t size) {
  Pro().exit_if_not_pro();
};

void ic0_msg_reply() { Pro().exit_if_not_pro(); };

void ic0_msg_reject(uintptr_t src, uint32_t size) { Pro().exit_if_not_pro(); };

uint64_t ic0_msg_funds_available(uintptr_t src, uint32_t size) {
  Pro().exit_if_not_pro();
  return 0;
};

uint64_t ic0_msg_funds_refunded(uintptr_t src, uint32_t size) {
  Pro().exit_if_not_pro();
  return 0;
};

void ic0_msg_funds_accept(uintptr_t src, uint32_t size, uint64_t amount) {
  Pro().exit_if_not_pro();
};

uint32_t ic0_canister_self_size() {
  Pro().exit_if_not_pro();
  return 0;
};

void ic0_canister_self_copy(uintptr_t dst, uint32_t off, uint32_t size) {
  Pro().exit_if_not_pro();
};

uint64_t ic0_canister_balance(uintptr_t src, uint32_t size) {
  Pro().exit_if_not_pro();
  return 0;
};

void ic0_call_new(uintptr_t callee_src, uint32_t callee_size,
                  uintptr_t name_src, uint32_t name_size, uintptr_t reply_fun,
                  uint32_t reply_env, uintptr_t reject_fun,
                  uint32_t reject_env) {
  Pro().exit_if_not_pro();
};

void ic0_call_on_cleanup(uintptr_t fun, uint32_t env) {
  Pro().exit_if_not_pro();
};

void ic0_call_data_append(uintptr_t src, uint32_t size) {
  Pro().exit_if_not_pro();
};

void ic0_call_funds_add(uintptr_t src, uint32_t size, uint64_t amount) {
  Pro().exit_if_not_pro();
};

uint32_t ic0_call_perform() {
  Pro().exit_if_not_pro();
  return 0;
};
uint32_t ic0_stable_size() {
  Pro().exit_if_not_pro();
  return 0;
};

uint32_t ic0_stable_grow(uint32_t new_pages) {
  Pro().exit_if_not_pro();
  return 0;
};

void ic0_stable_write(uint32_t off, uintptr_t src, uint32_t size) {
  Pro().exit_if_not_pro();
};

void ic0_stable_read(uintptr_t dst, uint32_t off, uint32_t size) {
  Pro().exit_if_not_pro();
};

uint64_t ic0_time() {
  Pro().exit_if_not_pro();
  return 0;
};

void ic0_debug_print(uintptr_t src, uint32_t size) {
  const char *str = (const char *)(void *)src;
  std::cout << str << std::endl;
};

void ic0_trap(uintptr_t src, uint32_t size) {
  std::string msg = "Mock IC trap: ";
  msg.append((const char *)(void *)src);

  if (!global_mockIC->get_silent_on_trap()) IC_API::debug_print(msg);

  throw std::runtime_error(msg);
};