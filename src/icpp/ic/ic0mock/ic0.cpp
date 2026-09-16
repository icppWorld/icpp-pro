// Mocked ic0 functions to mimic the IC.
// Use in a native executable, for development, debug & testing.

#include "ic0.h"
#include "candid_type.h"
#include "candid_type_all_includes.h"
#include "ic_api.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdexcept>

// Helpers & mock-only state for the 128-bit cycles + inspect-message APIs.
// Convention (same as the replica): a 128-bit amount passed IN arrives as a
// (high, low) uint64_t pair; an amount written OUT is 16 little-endian bytes
// at dst - the in-memory layout of a little-endian __uint128_t.
namespace {
void write_u128(uintptr_t dst, __uint128_t value) {
  memcpy(reinterpret_cast<uint8_t *>(dst), &value, sizeof(__uint128_t));
}

__uint128_t u128_from_parts(uint64_t high, uint64_t low) {
  return (static_cast<__uint128_t>(high) << 64) | low;
}

bool g_mock_accept_message_called{false};
__uint128_t g_mock_call_cycles_added{0};
} // namespace

#include "candid_type.h"
#include "candid_type_all_includes.h"
#include "global.h"
#include "vec_bytes.h"

// See:
// https://smartcontracts.org/docs/interface-spec/index.html#system-api-imports
uint32_t ic0_msg_arg_data_size() {
  return (uint32_t)global_mockIC->vec_in().size();
}

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

  uint8_t *bytes = reinterpret_cast<uint8_t *>(dst);
  const auto &vec_in = global_mockIC->vec_in();

  // Cast the std::byte elements to uint8_t while copying.
  std::transform(vec_in.begin(), vec_in.end(), bytes + off,
                 [](std::byte b) { return static_cast<uint8_t>(b); });
}

uint32_t ic0_msg_caller_size() {
  CandidTypePrincipal caller = global_mockIC->get_caller();
  return (uint32_t)caller.get_v_bytes().size();
}

void ic0_msg_caller_copy(uintptr_t dst, uint32_t off, uint32_t size) {
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
  std::cout << "ic0mock ic0::msg_reject_code" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
}

uint32_t ic0_msg_reject_msg_size() {
  std::cout << "ic0mock ic0::msg_reject_msg_size" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
}

void ic0_msg_reject_msg_copy(uintptr_t dst, uint32_t off, uint32_t size) {
  std::cout << "ic0mock ic0::msg_reject_msg_copy" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
}

void ic0_msg_reply_data_append(uintptr_t src, uint32_t size) {
  global_mockIC->msg_reply_data_append(src, size);
}

void ic0_msg_reply() {
#if ICPP_VERBOSE > 0
  IC_API::debug_print(
      "ic0mock ic0::msg_reply - content of m_didl_out in global_mockIC...");
  global_mockIC->get_msg_out().debug_print();
#endif // ICPP_VERBOSE
  global_mockIC->get_msg_out().trap_if_vec_does_not_start_with_DIDL();
}

void ic0_msg_reject(uintptr_t src, uint32_t size) {
  std::cout << "ic0mock ic0::msg_reject" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
}

uint32_t ic0_msg_method_name_size() {
  return (uint32_t)global_mockIC->get_msg_method_name().size();
}

void ic0_msg_method_name_copy(uintptr_t dst, uint32_t off, uint32_t size) {
  std::string name = global_mockIC->get_msg_method_name();
  memcpy(reinterpret_cast<uint8_t *>(dst), name.data() + off, size);
}

void ic0_accept_message() {
  // The replica traps when accept_message is called more than once.
  if (g_mock_accept_message_called) {
    IC_API::trap("ic0.accept_message: the function was already called.");
  }
  g_mock_accept_message_called = true;
}

uint32_t ic0_canister_self_size() {
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

void ic0_canister_cycle_balance128(uintptr_t dst) {
  __uint128_t canister_self_cycle_balance =
      global_mockIC->get_canister_self_cycle_balance();

  // Copy the __uint128_t value into the byte array
  memcpy(reinterpret_cast<uint8_t *>(dst), &canister_self_cycle_balance,
         sizeof(__uint128_t));
}

void ic0_canister_liquid_cycle_balance128(uintptr_t dst) {
  // Mock simplification: the liquid balance equals the total balance (the
  // mock has no freezing threshold or in-flight message reservations).
  write_u128(dst, global_mockIC->get_canister_self_cycle_balance());
}

void ic0_msg_cycles_available128(uintptr_t dst) {
  write_u128(dst, global_mockIC->get_msg_cycles_available());
}

void ic0_msg_cycles_refunded128(uintptr_t dst) {
  write_u128(dst, global_mockIC->get_msg_cycles_refunded());
}

void ic0_msg_cycles_accept128(uint64_t max_amount_high, uint64_t max_amount_low,
                              uintptr_t dst) {
  // accepted = min(max_amount, available); moves from available to balance.
  __uint128_t max_amount = u128_from_parts(max_amount_high, max_amount_low);
  __uint128_t available = global_mockIC->get_msg_cycles_available();
  __uint128_t accepted = std::min(max_amount, available);
  global_mockIC->set_msg_cycles_available(available - accepted);
  global_mockIC->add_cycles_balance(accepted);
  write_u128(dst, accepted);
}

void ic0_cycles_burn128(uint64_t amount_high, uint64_t amount_low,
                        uintptr_t dst) {
  // Burns no more than the balance; writes the amount actually burned.
  __uint128_t amount = u128_from_parts(amount_high, amount_low);
  __uint128_t balance = global_mockIC->get_canister_self_cycle_balance();
  __uint128_t burned = std::min(amount, balance);
  global_mockIC->sub_cycles_balance(burned);
  write_u128(dst, burned);
}

void ic0_call_cycles_add128(uint64_t amount_high, uint64_t amount_low) {
  g_mock_call_cycles_added += u128_from_parts(amount_high, amount_low);
}

void ic0_call_new(uintptr_t callee_src, uint32_t callee_size,
                  uintptr_t name_src, uint32_t name_size, uintptr_t reply_fun,
                  uint32_t reply_env, uintptr_t reject_fun,
                  uint32_t reject_env) {
  std::cout << "ic0mock ic0::call_new" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
}

void ic0_call_on_cleanup(uintptr_t fun, uint32_t env) {
  std::cout << "ic0mock ic0::call_on_cleanup" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
}

void ic0_call_data_append(uintptr_t src, uint32_t size) {
  std::cout << "ic0mock ic0::call_data_append" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
}

uint32_t ic0_call_perform() {
  std::cout << "ic0mock ic0::call_perform" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
}

uint32_t ic0_stable_size() {
  std::cout << "ic0mock ic0::stable_size" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
}

uint32_t ic0_stable_grow(uint32_t new_pages) {
  std::cout << "ic0mock ic0::stable_grow" << std::endl;
  std::cout << "...PATCH-PATCH- Returning 0..." << std::endl;
  return 0;
}

void ic0_stable_write(uint32_t off, uintptr_t src, uint32_t size) {
  std::cout << "ic0mock ic0::stable_write" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
}

void ic0_stable_read(uintptr_t dst, uint32_t off, uint32_t size) {
  std::cout << "ic0mock ic0::stable_read" << std::endl;
  std::cout << "...doing nothing..." << std::endl;
}

void ic0_certified_data_set(uintptr_t src, uint32_t size) {
  // The replica enforces CERTIFIED_DATA_MAX_LENGTH = 32.
  if (size > 32) {
    IC_API::trap("ic0.certified_data_set: payload too large (max 32 bytes). "
                 "Try certifying just the hash of your data.");
  }
  const uint8_t *p_bytes = reinterpret_cast<const uint8_t *>(src);
  global_mockIC->set_certified_data(
      std::vector<uint8_t>(p_bytes, p_bytes + size));
}

uint32_t ic0_data_certificate_present() {
  return global_mockIC->get_data_certificate_present() ? 1 : 0;
}

uint32_t ic0_data_certificate_size() {
  return (uint32_t)global_mockIC->get_data_certificate().size();
}

void ic0_data_certificate_copy(uintptr_t dst, uint32_t off, uint32_t size) {
  std::vector<uint8_t> cert = global_mockIC->get_data_certificate();
  memcpy(reinterpret_cast<uint8_t *>(dst), cert.data() + off, size);
}

// Mock-IC-only state: last armed global-timer deadline, an invocation
// counter so tests can prove arm_next() actually issued an ic0 syscall, and
// a time override (flag + value pair, so clearing flips the flag without
// silently pinning time at 0). All exposed via the accessor / control
// helpers further below; native test drivers use them to drive scenarios
// that wall-clock alone would undermine.
namespace {
uint64_t g_mock_armed_global_timer{0};
uint64_t g_mock_global_timer_set_call_count{0};
bool g_mock_time_overridden{false};
uint64_t g_mock_time_override_ns{0};
} // namespace

uint64_t ic0_time() {
  if (g_mock_time_overridden) {
    return g_mock_time_override_ns;
  }
  // steady_clock, not high_resolution_clock: the latter's is_steady is
  // implementation-defined and on some libc++ targets aliases system_clock,
  // which can move backward when the wall clock is adjusted. Native timer
  // tests assume monotonicity (deadline math relies on it), so use the
  // clock that is_steady is guaranteed to be true for.
  uint64_t time_in_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                            std::chrono::steady_clock::now().time_since_epoch())
                            .count();
  return time_in_ns;
}

uint64_t ic0_global_timer_set(uint64_t timestamp_ns) {
  uint64_t previous = g_mock_armed_global_timer;
  g_mock_armed_global_timer = timestamp_ns;
  ++g_mock_global_timer_set_call_count;
#if ICPP_VERBOSE > 0
  std::cout << "ic0mock ic0::global_timer_set -> " << timestamp_ns << " (was "
            << previous << ")" << std::endl;
#endif
  return previous;
}

uint64_t ic0mock_global_timer_set_call_count() {
  return g_mock_global_timer_set_call_count;
}

void ic0mock_set_time_override(uint64_t time_ns) {
  g_mock_time_overridden = true;
  g_mock_time_override_ns = time_ns;
}

void ic0mock_clear_time_override() {
  // Flip the flag only; do not zero the override value. If a subsequent test
  // forgot to call set_override and we had reset the value to 0, ic0_time()
  // would silently pin to 0 instead of returning wall-clock.
  g_mock_time_overridden = false;
}

bool ic0mock_accept_message_called() { return g_mock_accept_message_called; }

void ic0mock_clear_accept_message() { g_mock_accept_message_called = false; }

__uint128_t ic0mock_call_cycles_added() { return g_mock_call_cycles_added; }

void ic0mock_clear_call_cycles_added() { g_mock_call_cycles_added = 0; }

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