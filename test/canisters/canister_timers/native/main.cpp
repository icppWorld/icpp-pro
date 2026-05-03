// Native test driver for the canister_timers test canister.
// Build with: `icpp build-native` and run `./build-native/mockic.exe`.
//
// We exercise the timer-dispatch path by calling canister_global_timer
// directly through MockIC, since the Mock IC does not actually fire timers
// on its own. Counter state is read via the extern globals declared in
// my_canister.h to avoid candid round-tripping.

#include "main.h"

#include <iostream>

// ic_api.h transitively pulls in canister.h, which declares the built-in
// canister_global_timer() entry. Include it before the user's canister
// header so the canister_*.h forward-declaration chain is fully resolved.
#include "ic_api.h"
#include "ic_timers.h"
#include "icpp_hooks.h"
#include "mock_ic.h"

#include "../src/my_canister.h"

namespace {

// Minimal candid for "()" (DIDL header, 0 types, 0 args).
constexpr const char *EMPTY_CANDID = "4449444c0000";

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
  bool exit_on_fail = true;
  MockIC mockIC(exit_on_fail);

  std::string my_principal{
      "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae"};
  bool silent_on_trap = true;

  int extra_failures = 0;

  // Pass empty candid_out_expected (""), which makes MockIC accept whatever
  // the function writes back. We validate state directly via the extern
  // globals and IcTimers::instance().size().
  mockIC.run_test("canister_init", canister_init, EMPTY_CANDID, "",
                  silent_on_trap, my_principal);

  // ---- one-shot fires after a dispatch tick ----------------------------
  mockIC.run_test("start_one_shot", start_one_shot, EMPTY_CANDID, "",
                  silent_on_trap, my_principal);
  extra_failures += expect_eq_u64("registry size after start_one_shot",
                                  IcTimers::instance().size(), 1);

  mockIC.run_test("canister_global_timer (drain one-shot)",
                  canister_global_timer, EMPTY_CANDID, "", silent_on_trap,
                  my_principal);
  extra_failures += expect_eq_u64("g_one_shot_count", g_one_shot_count, 1);
  extra_failures += expect_eq_u64("registry size after dispatch",
                                  IcTimers::instance().size(), 0);

  // ---- a second dispatch with no due timers is a no-op -----------------
  mockIC.run_test("canister_global_timer (no-op when empty)",
                  canister_global_timer, EMPTY_CANDID, "", silent_on_trap,
                  my_principal);
  extra_failures +=
      expect_eq_u64("g_one_shot_count after no-op", g_one_shot_count, 1);

  // ---- cancel removes the timer without firing -------------------------
  // Add a one-shot, capture the id, cancel it, dispatch — counter unchanged.
  uint64_t to_cancel = IC_API::set_timer(0, []() { ++g_one_shot_count; });
  extra_failures += expect_eq_u64("registry size after manual set_timer",
                                  IcTimers::instance().size(), 1);
  bool ok = IC_API::cancel_timer(to_cancel);
  if (!ok) {
    std::cout << "FAIL: cancel_timer returned false" << std::endl;
    ++extra_failures;
  } else {
    std::cout << "PASS: cancel_timer returned true" << std::endl;
  }
  extra_failures += expect_eq_u64("registry size after cancel",
                                  IcTimers::instance().size(), 0);

  mockIC.run_test("canister_global_timer (after cancel)", canister_global_timer,
                  EMPTY_CANDID, "", silent_on_trap, my_principal);
  extra_failures +=
      expect_eq_u64("g_one_shot_count after cancel", g_one_shot_count, 1);

  // Cancelling an unknown id returns false.
  if (IC_API::cancel_timer(99999)) {
    std::cout << "FAIL: cancel_timer(99999) should return false" << std::endl;
    ++extra_failures;
  } else {
    std::cout << "PASS: cancel_timer(unknown) returns false" << std::endl;
  }

  // ---- many one-shots fire in deadline order, capped at 10 per tick ----
  for (int i = 0; i < 15; ++i) {
    IC_API::set_timer(0, []() { ++g_one_shot_count; });
  }
  extra_failures += expect_eq_u64("registry size with 15 timers",
                                  IcTimers::instance().size(), 15);
  uint64_t before = g_one_shot_count;

  mockIC.run_test("canister_global_timer (drain 10 of 15)",
                  canister_global_timer, EMPTY_CANDID, "", silent_on_trap,
                  my_principal);
  extra_failures += expect_eq_u64("g_one_shot_count after first drain",
                                  g_one_shot_count, before + 10);
  extra_failures +=
      expect_eq_u64("registry size after first drain (5 remaining)",
                    IcTimers::instance().size(), 5);

  mockIC.run_test("canister_global_timer (drain remaining 5)",
                  canister_global_timer, EMPTY_CANDID, "", silent_on_trap,
                  my_principal);
  extra_failures += expect_eq_u64("g_one_shot_count after second drain",
                                  g_one_shot_count, before + 15);
  extra_failures += expect_eq_u64("registry empty after second drain",
                                  IcTimers::instance().size(), 0);

  std::cout << "\n----------\n";
  std::cout << "Extra direct-state assertions: "
            << (extra_failures == 0 ? "all PASSED" : "some FAILED")
            << std::endl;

  int summary_rc = mockIC.test_summary();
  return summary_rc != 0 ? summary_rc : (extra_failures > 0 ? 1 : 0);
}
