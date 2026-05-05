// Native test driver for the canister_user_timer_override smoke canister.
//
// Proves that a user-supplied strong canister_global_timer overrides the
// icpp-pro built-in weak dispatcher. The test:
//   1. Registers a recurring IcTimers timer (bumps g_icpp_callback_calls
//      if the icpp-pro dispatcher ever runs).
//   2. Drives canister_global_timer once via MockIC.
//   3. Asserts the user's counter advanced AND the icpp-pro callback
//      counter stayed at 0 AND the icpp-pro timer registry size stayed at
//      1 (the recurring timer was not drained).

#include "main.h"

#include <iostream>

#include "ic_api.h"
#include "ic_timers.h"
#include "icpp_hooks.h"
#include "mock_ic.h"

#include "../src/my_canister.h"

namespace {

constexpr const char *EMPTY_CANDID = "4449444c0000";

// Candid for `(period_ns: nat64)` with period = 1_000_000_000 ns (1 s).
// 4449444c -> "DIDL" magic, 00 -> 0 type-table entries, 01 -> 1 arg, 78 ->
// nat64 type, then 8 bytes little-endian = 1_000_000_000.
constexpr const char *CANDID_NAT64_1S = "4449444c00017800ca9a3b00000000";

int expect_eq_u64(const char *label, uint64_t actual, uint64_t expected) {
  if (actual != expected) {
    std::cout << "FAIL: " << label << " expected " << expected << ", got "
              << actual << std::endl;
    return 1;
  }
  std::cout << "PASS: " << label << " == " << actual << std::endl;
  return 0;
}

} // namespace

int main() {
  bool exit_on_fail = false;
  MockIC mockIC(exit_on_fail);

  std::string my_principal{
      "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae"};
  bool silent_on_trap = true;
  int extra_failures = 0;

  mockIC.run_test("canister_init", canister_init, EMPTY_CANDID, "",
                  silent_on_trap, my_principal);

  // Register a recurring IcTimers timer with period 1s. Whether this
  // callback ever fires depends on whether the icpp-pro built-in dispatcher
  // runs. Below we drive canister_global_timer manually; if the user's
  // strong override won at link time, only g_user_dispatcher_calls will
  // advance, not g_icpp_callback_calls.
  mockIC.run_test("start_user_timer", start_user_timer, CANDID_NAT64_1S, "",
                  silent_on_trap, my_principal);
  extra_failures += expect_eq_u64("registry size after start_user_timer",
                                  IcTimers::instance().size(), 1);
  extra_failures += expect_eq_u64("g_user_dispatcher_calls before dispatch",
                                  g_user_dispatcher_calls, 0);
  extra_failures += expect_eq_u64("g_icpp_callback_calls before dispatch",
                                  g_icpp_callback_calls, 0);

  // Drive canister_global_timer. With the override in place, this resolves
  // to my_canister.cpp::canister_global_timer (strong) — NOT the icpp-pro
  // weak dispatcher.
  mockIC.run_test("canister_global_timer (user override)",
                  canister_global_timer, EMPTY_CANDID, "", silent_on_trap,
                  my_principal);

  // The defining assertions:
  extra_failures += expect_eq_u64(
      "g_user_dispatcher_calls after dispatch (user override ran)",
      g_user_dispatcher_calls, 1);
  extra_failures += expect_eq_u64(
      "g_icpp_callback_calls after dispatch (icpp-pro dispatcher did NOT run)",
      g_icpp_callback_calls, 0);
  extra_failures += expect_eq_u64(
      "registry size after dispatch (icpp-pro registry not drained)",
      IcTimers::instance().size(), 1);

  // Drive again to confirm repeated invocation also resolves to user.
  mockIC.run_test("canister_global_timer (user override, second call)",
                  canister_global_timer, EMPTY_CANDID, "", silent_on_trap,
                  my_principal);
  extra_failures +=
      expect_eq_u64("g_user_dispatcher_calls after second dispatch",
                    g_user_dispatcher_calls, 2);
  extra_failures +=
      expect_eq_u64("g_icpp_callback_calls still 0 after second dispatch",
                    g_icpp_callback_calls, 0);
  extra_failures += expect_eq_u64("registry size still 1 after second dispatch",
                                  IcTimers::instance().size(), 1);

  std::cout << "\n----------\n";
  std::cout << "Extra direct-state assertions: "
            << (extra_failures == 0 ? "all PASSED" : "some FAILED")
            << std::endl;

  int summary_rc = mockIC.test_summary();
  return summary_rc != 0 ? summary_rc : (extra_failures > 0 ? 1 : 0);
}
