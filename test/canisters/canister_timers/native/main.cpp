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

// In the native build the include path resolves "ic0.h" to the ic0mock
// header, which exposes the mock-only test helpers
// (ic0mock_global_timer_set_call_count, ic0mock_set_time_override,
// ic0mock_clear_time_override) used by the cache-stranding regression below.
#include "ic0.h"

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
  // exit_on_fail = false so a failing run_test does not call exit(1) and
  // bypass the cleanup code below (in particular, ic0mock_clear_time_override
  // calls in the time-pinned regression scenario). Failures are aggregated
  // via mockIC.test_summary() + extra_failures; main() returns the OR of
  // both at the end.
  bool exit_on_fail = false;
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

  // ---- Cache-stranding regression (the High #1 bug from the rev1 review) --
  // Without time-override, 15 successive set_timer(0,...) calls produce 15
  // different deadlines (wall-clock advances between them) and the bug's
  // exact pre-condition (remaining earliest deadline equals the
  // previously-armed deadline) does not hold. We pin time so all 15 land at
  // the same deadline, then dispatch and assert that arm_next() actually
  // re-armed at the same deadline value. Without the fix in
  // IcTimers::dispatch_due (which sets m_armed_deadline = 0 at entry),
  // arm_next() would skip the syscall and the remaining 5 timers would be
  // permanently stranded.
  ic0mock_set_time_override(1000);
  uint64_t cnt_before = ic0mock_global_timer_set_call_count();
  uint64_t shots_before = g_one_shot_count;
  for (int i = 0; i < 15; ++i) {
    IC_API::set_timer(0, []() { ++g_one_shot_count; });
  }
  extra_failures +=
      expect_eq_u64("[pinned] registry size before dispatch with 15 timers",
                    IcTimers::instance().size(), 15);

  // Advance the override past the deadline (which is 1000 + 0 = 1000).
  ic0mock_set_time_override(2000);

  // First dispatch: drain 10 of 15.
  uint64_t cnt_a = ic0mock_global_timer_set_call_count();
  mockIC.run_test("[pinned] canister_global_timer (drain 10 of 15)",
                  canister_global_timer, EMPTY_CANDID, "", silent_on_trap,
                  my_principal);
  uint64_t cnt_b = ic0mock_global_timer_set_call_count();
  extra_failures += expect_eq_u64(
      "[pinned] ic0_global_timer_set call delta after first dispatch",
      cnt_b - cnt_a, 1);
  extra_failures +=
      expect_eq_u64("[pinned] g_one_shot_count delta after first dispatch",
                    g_one_shot_count - shots_before, 10);
  extra_failures +=
      expect_eq_u64("[pinned] registry size after first dispatch (5 left)",
                    IcTimers::instance().size(), 5);

  // Second dispatch: drain remaining 5 (under the 10-per-tick cap). Note
  // we expect a delta of 0 here, not 1. After draining, the registry is
  // empty, so arm_next() computes target = 0 ("disarm"). The cache fix
  // also set m_armed_deadline = 0 at function entry, so target == cache
  // and arm_next legitimately skips the syscall — disarming an already-
  // disarmed IC is wasteful. The interesting assertion is the FIRST
  // dispatch's delta above (which would be 0 without the cache fix and
  // is 1 with it).
  uint64_t cnt_c = ic0mock_global_timer_set_call_count();
  mockIC.run_test("[pinned] canister_global_timer (drain remaining 5)",
                  canister_global_timer, EMPTY_CANDID, "", silent_on_trap,
                  my_principal);
  uint64_t cnt_d = ic0mock_global_timer_set_call_count();
  extra_failures += expect_eq_u64(
      "[pinned] ic0_global_timer_set call delta after second dispatch (registry empty -> already disarmed -> skip)",
      cnt_d - cnt_c, 0);
  extra_failures += expect_eq_u64("[pinned] g_one_shot_count delta total",
                                  g_one_shot_count, shots_before + 15);
  extra_failures +=
      expect_eq_u64("[pinned] registry empty after second dispatch",
                    IcTimers::instance().size(), 0);

  // Total syscall count for the whole scenario: one when the first set_timer
  // armed the IC at deadline 1000, plus one when the first dispatch re-armed
  // at the same deadline 1000 (the cache fix forced this through). The second
  // dispatch contributes 0 (see comment above). Total: 2.
  extra_failures +=
      expect_eq_u64("[pinned] ic0_global_timer_set total scenario delta",
                    ic0mock_global_timer_set_call_count() - cnt_before, 2);

  // Always clear the override before main() returns. Pattern A (this whole
  // runner uses exit_on_fail=false) means we reach this line even on
  // assertion failure, so a single clear at scenario end is sufficient.
  ic0mock_clear_time_override();

  // ---- Cancel-all-from-callback test (commit 4) -------------------------
  // Locks down the documented current-dispatch semantics: callbacks already
  // collected into the in-flight dispatch_due() batch (copied into the
  // local `to_run` vector before any callback ran) STILL execute even after
  // a callback calls cancel_all_timers() mid-batch. Future firings, and
  // any recurring timer's next-deadline rescheduling, are cancelled.
  //
  // Time is pinned (per reviewer) so all 3 recurring timers compute the
  // same deadline and are guaranteed due in a single dispatch batch — wall-
  // clock drift between successive set_timer_recurring calls would
  // otherwise give them slightly different deadlines.
  //
  // Registration order is the test's intent: the cancelling timer is
  // registered FIRST so its callback lands at the front of `to_run`. We
  // want to prove that the OTHER two callbacks, copied into `to_run`
  // before the cancel ran, still execute. Registering the cancelling
  // timer last would prove nothing about post-cancel callback execution.
  ic0mock_set_time_override(1000);
  // Reset counters from earlier scenarios so the asserts below are scoped.
  IC_API::cancel_all_timers();
  uint64_t t1_count = 0, t2_count = 0, t3_count = 0;
  IC_API::set_timer_recurring(100, [&t1_count]() {
    ++t1_count;
    IC_API::cancel_all_timers();
  });
  IC_API::set_timer_recurring(100, [&t2_count]() { ++t2_count; });
  IC_API::set_timer_recurring(100, [&t3_count]() { ++t3_count; });
  extra_failures += expect_eq_u64(
      "[cancel-from-cb] registry size after registering 3 recurring",
      IcTimers::instance().size(), 3);

  // Advance past the deadline (1000 + 100 = 1100).
  ic0mock_set_time_override(1100);
  uint64_t cnt_e = ic0mock_global_timer_set_call_count();
  mockIC.run_test("[cancel-from-cb] canister_global_timer",
                  canister_global_timer, EMPTY_CANDID, "", silent_on_trap,
                  my_principal);
  uint64_t cnt_f = ic0mock_global_timer_set_call_count();

  extra_failures +=
      expect_eq_u64("[cancel-from-cb] T1 callback ran", t1_count, 1);
  extra_failures += expect_eq_u64(
      "[cancel-from-cb] T2 callback ran despite earlier cancel_all_timers",
      t2_count, 1);
  extra_failures += expect_eq_u64(
      "[cancel-from-cb] T3 callback ran despite earlier cancel_all_timers",
      t3_count, 1);
  extra_failures +=
      expect_eq_u64("[cancel-from-cb] registry empty after dispatch",
                    IcTimers::instance().size(), 0);
  // The clear()'s unconditional ic0_global_timer_set(0) should show in the
  // mock counter delta during this dispatch. The dispatch's final arm_next
  // sees an empty registry (target=0) matching the cache (also 0 after the
  // top-of-dispatch invalidation), so it skips. Net delta: 1 syscall from
  // clear()'s explicit disarm.
  extra_failures += expect_eq_u64(
      "[cancel-from-cb] ic0_global_timer_set delta (clear's disarm)",
      cnt_f - cnt_e, 1);

  // Drive again — registry is empty so no callbacks should fire.
  mockIC.run_test("[cancel-from-cb] canister_global_timer (second, no-op)",
                  canister_global_timer, EMPTY_CANDID, "", silent_on_trap,
                  my_principal);
  extra_failures +=
      expect_eq_u64("[cancel-from-cb] T1 did not fire again", t1_count, 1);
  extra_failures +=
      expect_eq_u64("[cancel-from-cb] T2 did not fire again", t2_count, 1);
  extra_failures +=
      expect_eq_u64("[cancel-from-cb] T3 did not fire again", t3_count, 1);

  ic0mock_clear_time_override();

  std::cout << "\n----------\n";
  std::cout << "Extra direct-state assertions: "
            << (extra_failures == 0 ? "all PASSED" : "some FAILED")
            << std::endl;

  int summary_rc = mockIC.test_summary();
  return summary_rc != 0 ? summary_rc : (extra_failures > 0 ? 1 : 0);
}
