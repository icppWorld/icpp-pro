// Mocked ic0 functions to mimic the IC.
// Use in a native executable, for development, debug & testing.

#include "ic0.h"
#include "candid_type.h"
#include "candid_type_all_includes.h"
#include "ic_api.h"
#include "pro.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "candid_type.h"
#include "candid_type_all_includes.h"
#include "global.h"
#include "vec_bytes.h"

// See:
// https://smartcontracts.org/docs/interface-spec/index.html#system-api-imports
uint32_t ic0_msg_arg_data_size() {
  Pro().exit_if_not_pro();
  return (uint32_t)global_mockIC->vec_in().size();
}

void ic0_msg_arg_data_copy(uintptr_t dst, uint32_t off, uint32_t size) {
  Pro().exit_if_not_pro();
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

  uint8_t *bytes = reinterpret_cast<uint8_t *>(dst);
  const auto &vec_in = global_mockIC->vec_in();

  // Cast the std::byte elements to uint8_t while copying.
  std::transform(vec_in.begin(), vec_in.end(), bytes + off,
                 [](std::byte b) { return static_cast<uint8_t>(b); });
}

uint32_t ic0_msg_caller_size() {
  Pro().exit_if_not_pro();
  CandidTypePrincipal caller = global_mockIC->get_caller();
  return (uint32_t)caller.get_v_bytes().size();
}

void ic0_msg_caller_copy(uintptr_t dst, uint32_t off, uint32_t size) {
  Pro().exit_if_not_pro();
  CandidTypePrincipal caller = global_mockIC->get_caller();

  if (size != (uint32_t)caller.get_v_bytes().size()) {
    std::cout
        << "TEST PROGRAM ERROR: ic0_msg_caller_copy in the mockIC has wrong size"
        << std::endl;
    std::cout << " size argument = " << size << std::endl;
    std::cout << " size mockIC   = " << (uint32_t)caller.get_v_bytes().size()
              << std::endl;
    abort();
  }

  uint8_t *bytes = reinterpret_cast<uint8_t *>(dst);
  const std::vector<uint8_t> &caller_bytes = caller.get_v_bytes().vec_uint8_t();
  std::copy(caller_bytes.begin(), caller_bytes.end(), bytes + off);
}

uint32_t ic0_msg_reject_code() {
  Pro().exit_if_not_pro();
  std::cout << "ic0mock ic0::msg_reject_code" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
}

uint32_t ic0_msg_reject_msg_size() {
  Pro().exit_if_not_pro();
  std::cout << "ic0mock ic0::msg_reject_msg_size" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
}

void ic0_msg_reject_msg_copy(uintptr_t dst, uint32_t off, uint32_t size) {
  Pro().exit_if_not_pro();
  std::cout << "ic0mock ic0::msg_reject_msg_copy" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
}

void ic0_msg_reply_data_append(uintptr_t src, uint32_t size) {
  Pro().exit_if_not_pro();
  global_mockIC->msg_reply_data_append(src, size);
}

void ic0_msg_reply() {
  Pro().exit_if_not_pro();
#if ICPP_VERBOSE > 0
  IC_API::debug_print(
      "ic0mock ic0::msg_reply - content of m_didl_out in global_mockIC...");
  global_mockIC->get_msg_out().debug_print();
#endif // ICPP_VERBOSE
  global_mockIC->get_msg_out().trap_if_vec_does_not_start_with_DIDL();
}

void ic0_msg_reject(uintptr_t src, uint32_t size) {
  Pro().exit_if_not_pro();
  std::cout << "ic0mock ic0::msg_reject" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
}

uint64_t ic0_msg_funds_available(uintptr_t src, uint32_t size) {
  Pro().exit_if_not_pro();
  std::cout << "ic0mock ic0::msg_funds_available" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
}

uint64_t ic0_msg_funds_refunded(uintptr_t src, uint32_t size) {
  Pro().exit_if_not_pro();
  std::cout << "ic0mock ic0::msg_funds_refunded" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
}

void ic0_msg_funds_accept(uintptr_t src, uint32_t size, uint64_t amount) {
  Pro().exit_if_not_pro();
  std::cout << "ic0mock ic0::msg_funds_accept" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
}

uint32_t ic0_canister_self_size() {
  Pro().exit_if_not_pro();
  CandidTypePrincipal canister_self = global_mockIC->get_canister_self();
  return (uint32_t)canister_self.get_v_bytes().size();
}

void ic0_canister_self_copy(uintptr_t dst, uint32_t off, uint32_t size) {
  CandidTypePrincipal canister_self = global_mockIC->get_canister_self();

  if (size != (uint32_t)canister_self.get_v_bytes().vec_uint8_t().size()) {
    std::cout
        << "TEST PROGRAM ERROR: ic0_canister_self_copy in the mockIC has wrong size"
        << std::endl;
    std::cout << " size argument = " << size << std::endl;
    std::cout << " size mockIC   = "
              << (uint32_t)canister_self.get_v_bytes().vec_uint8_t().size()
              << std::endl;
    abort();
  }

  uint8_t *bytes = reinterpret_cast<uint8_t *>(dst);
  const std::vector<uint8_t> &canister_self_bytes =
      canister_self.get_v_bytes().vec_uint8_t();
  std::copy(canister_self_bytes.begin(), canister_self_bytes.end(),
            bytes + off);
}

uint64_t ic0_canister_balance(uintptr_t src, uint32_t size) {
  Pro().exit_if_not_pro();
  std::cout << "ic0mock ic0::canister_balance" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
}

void ic0_canister_cycle_balance128(uintptr_t dst) {
  Pro().exit_if_not_pro();
  __uint128_t canister_self_cycle_balance =
      global_mockIC->get_canister_self_cycle_balance();

  // Copy the __uint128_t value into the byte array
  memcpy(reinterpret_cast<uint8_t *>(dst), &canister_self_cycle_balance,
         sizeof(__uint128_t));
}

void ic0_call_new(uintptr_t callee_src, uint32_t callee_size,
                  uintptr_t name_src, uint32_t name_size, uintptr_t reply_fun,
                  uint32_t reply_env, uintptr_t reject_fun,
                  uint32_t reject_env) {
  Pro().exit_if_not_pro();
  std::cout << "ic0mock ic0::call_new" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
}

void ic0_call_on_cleanup(uintptr_t fun, uint32_t env) {
  Pro().exit_if_not_pro();
  std::cout << "ic0mock ic0::call_on_cleanup" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
}

void ic0_call_data_append(uintptr_t src, uint32_t size) {
  Pro().exit_if_not_pro();
  std::cout << "ic0mock ic0::call_data_append" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
}

void ic0_call_funds_add(uintptr_t src, uint32_t size, uint64_t amount) {
  Pro().exit_if_not_pro();
  std::cout << "ic0mock ic0::call_funds_add" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
}

uint32_t ic0_call_perform() {
  Pro().exit_if_not_pro();
  std::cout << "ic0mock ic0::call_perform" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
}

uint32_t ic0_stable_size() {
  Pro().exit_if_not_pro();
  std::cout << "ic0mock ic0::stable_size" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
}

uint32_t ic0_stable_grow(uint32_t new_pages) {
  Pro().exit_if_not_pro();
  std::cout << "ic0mock ic0::stable_grow" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
}

void ic0_stable_write(uint32_t off, uintptr_t src, uint32_t size) {
  Pro().exit_if_not_pro();
  std::cout << "ic0mock ic0::stable_write" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
}

void ic0_stable_read(uintptr_t dst, uint32_t off, uint32_t size) {
  Pro().exit_if_not_pro();
  std::cout << "ic0mock ic0::stable_read" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
}

uint64_t ic0_time() {
  Pro().exit_if_not_pro();
  uint64_t time_in_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();
  return time_in_ns;
}

uint32_t ic0_is_controller(uintptr_t src, uint32_t size) {
  const uint8_t *p_bytes = reinterpret_cast<const uint8_t *>(src);
  std::vector<uint8_t> bytes(p_bytes, p_bytes + size);
  CandidTypePrincipal principal(bytes);
  return std::string_view(principal.get_text()) == MOCKIC_CONTROLLER;
}

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
}

// Optionally prints the trap message, and always throws a std::runtime_error(msg)
void ic0_trap(uintptr_t src, uint32_t size) {
  std::string msg = "Mock IC trap: ";
  msg.append((const char *)(void *)src);

  if (!global_mockIC->get_silent_on_trap()) IC_API::debug_print(msg);

  throw std::runtime_error(msg);
}