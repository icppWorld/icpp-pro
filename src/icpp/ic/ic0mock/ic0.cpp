// Mocked ic0 functions to mimic the IC.
// Use in a native executable, for development, debug & testing.

#include "ic0.h"

#include <cstddef>
#include <cstdint>

#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "global.h"
#include "mock_ic.h"

// See:
// https://smartcontracts.org/docs/interface-spec/index.html#system-api-imports
uint32_t ic0_msg_arg_data_size() {
  return (uint32_t)global_mockIC->vec_in().size();
};

void ic0_msg_arg_data_copy(uintptr_t dst, uint32_t off, uint32_t size) {
  // See: https://stackoverflow.com/q/34291377/5480536
  // for an explanation of the uintptr_t conversion over (void *)

  if (size != (uint32_t)global_mockIC->vec_in().size()) {
    std::cout << "TEST PROGRAM ERROR: msg_in for mockIC has wrong size"
              << std::endl;
    std::cout << " size argument = " << size << std::endl;
    std::cout << " size mockIC   = " << (uint32_t)global_mockIC->vec_in().size()
              << std::endl;
    abort();
  }

  uint32_t off_now = off;
  uint8_t *bytes = (uint8_t *)(void *)dst;
  for (std::byte b : global_mockIC->vec_in()) {
    // The byte contains the decimal representation. Store it in an int
    // variable.
    int n = (int)b;

    // The IC works with a byte stored as (uint8_t). Cast it to that.
    *(bytes + off_now) = (uint8_t)n;
    off_now += 1;
  }
};

uint32_t ic0_msg_caller_size() {
  std::cout << "ic0mock ic0::msg_caller_size" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
};

void ic0_msg_caller_copy(uintptr_t dst, uint32_t off, uint32_t size) {
  std::cout << "ic0mock ic0::msg_caller_copy" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
};

uint32_t ic0_msg_reject_code() {
  std::cout << "ic0mock ic0::msg_reject_code" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
};

uint32_t ic0_msg_reject_msg_size() {
  std::cout << "ic0mock ic0::msg_reject_msg_size" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
};

void ic0_msg_reject_msg_copy(uintptr_t dst, uint32_t off, uint32_t size) {
  std::cout << "ic0mock ic0::msg_reject_msg_copy" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
};

void ic0_msg_reply_data_append(uintptr_t src, uint32_t size) {
  global_mockIC->msg_reply_data_append(src, size);
};

void ic0_msg_reply() {
#if ICPP_VERBOSE > 0
  IC_API::debug_print(
      "ic0mock ic0::msg_reply - content of m_didl_out in global_mockIC...");
  global_mockIC->get_msg_out().debug_print();
#endif // ICPP_VERBOSE
  global_mockIC->get_msg_out().trap_if_vec_does_not_start_with_DIDL();
};

void ic0_msg_reject(uintptr_t src, uint32_t size) {
  std::cout << "ic0mock ic0::msg_reject" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
};

uint64_t ic0_msg_funds_available(uintptr_t src, uint32_t size) {
  std::cout << "ic0mock ic0::msg_funds_available" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
};

uint64_t ic0_msg_funds_refunded(uintptr_t src, uint32_t size) {
  std::cout << "ic0mock ic0::msg_funds_refunded" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
};

void ic0_msg_funds_accept(uintptr_t src, uint32_t size, uint64_t amount) {
  std::cout << "ic0mock ic0::msg_funds_accept" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
};

uint32_t ic0_canister_self_size() {
  std::cout << "ic0mock ic0::canister_self_size" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
};

void ic0_canister_self_copy(uintptr_t dst, uint32_t off, uint32_t size) {
  std::cout << "ic0mock ic0::canister_self_copy" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
};

uint64_t ic0_canister_balance(uintptr_t src, uint32_t size) {
  std::cout << "ic0mock ic0::canister_balance" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
};

void ic0_call_new(uintptr_t callee_src, uint32_t callee_size,
                  uintptr_t name_src, uint32_t name_size, uintptr_t reply_fun,
                  uint32_t reply_env, uintptr_t reject_fun,
                  uint32_t reject_env) {
  std::cout << "ic0mock ic0::call_new" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
};

void ic0_call_on_cleanup(uintptr_t fun, uint32_t env) {
  std::cout << "ic0mock ic0::call_on_cleanup" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
};

void ic0_call_data_append(uintptr_t src, uint32_t size) {
  std::cout << "ic0mock ic0::call_data_append" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
};

void ic0_call_funds_add(uintptr_t src, uint32_t size, uint64_t amount) {
  std::cout << "ic0mock ic0::call_funds_add" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
};

uint32_t ic0_call_perform() {
  std::cout << "ic0mock ic0::call_perform" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
};
uint32_t ic0_stable_size() {
  std::cout << "ic0mock ic0::stable_size" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
};

uint32_t ic0_stable_grow(uint32_t new_pages) {
  std::cout << "ic0mock ic0::stable_grow" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
};

void ic0_stable_write(uint32_t off, uintptr_t src, uint32_t size) {
  std::cout << "ic0mock ic0::stable_write" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
};

void ic0_stable_read(uintptr_t dst, uint32_t off, uint32_t size) {
  std::cout << "ic0mock ic0::stable_read" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
};

uint64_t ic0_time() {
  std::cout << "ic0mock ic0::time" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
};

void ic0_debug_print(uintptr_t src, uint32_t size) {

  // Parameter list uses `(uintptr_t)src` instead of `(uint8_t)message`:
  // -> Either way works in IC canister, because target=wasm32 uses 32 bit
  // pointers.
  // -> uintptr_t is portable, and also works for native executable with 64 bit
  // pointers.

  // Reference:
  // - https:
  // //wiki.sei.cmu.edu/confluence/display/c/INT36-C.+Converting+a+pointer+to+integer+or+integer+to+pointer

  const char *str = (const char *)(void *)src;
  std::cout << str << std::endl;
};

void ic0_trap(uintptr_t src, uint32_t size) {
  // ic0mock IC throws a std::runtime_error(\"trap: msg\")...
  std::string msg = "Mock IC trap: ";
  msg.append((const char *)(void *)src);

  IC_API::debug_print(msg);

  throw std::runtime_error(msg);
};