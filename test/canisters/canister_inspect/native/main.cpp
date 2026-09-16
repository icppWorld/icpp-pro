// Main entry point for a native debug executable.
// Build it with: `icpp build-native` from the parent folder where 'icpp.toml'
// resides.
//
// canister_inspect_message is not something MockIC can trigger, so this
// driver calls it directly (the canister_timers precedent) with the mock's
// msg-method-name set, and asserts the accept flag via the ic0mock helpers.

#include "main.h"

#include <iostream>
#include <string>

#include "../src/my_canister.h"

// The Mock IC
// clang-format off
#include "ic_api.h"
#include "icpp_hooks.h"
#include "mock_ic.h"
#include "ic0.h" // resolves to the ic0mock header in a native build; brings
                 // the ic0mock_* test helpers (accept-message flag)
// clang-format on

// Must trap: accept_message called twice in one inspection.
static void native_double_accept() {
  IC_API ic_api(CanisterInspectMessage{"native_double_accept"}, false);
  ic_api.accept_message();
  ic_api.accept_message();
}

// Must trap: accept_message is only available in inspect_message entries.
static void native_accept_from_update() {
  IC_API ic_api(CanisterUpdate{"native_accept_from_update"}, false);
  ic_api.accept_message();
}

// Must trap: so is get_msg_method_name.
static void native_method_name_from_update() {
  IC_API ic_api(CanisterUpdate{"native_method_name_from_update"}, false);
  (void)ic_api.get_msg_method_name();
}

int main() {
  bool exit_on_fail = true;
  MockIC mockIC(exit_on_fail);

  std::string my_principal{
      "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae"};

  bool silent_on_trap = true;
  int extra_failures = 0;

  // An "allowed_" method must be accepted
  mockIC.set_msg_method_name("allowed_greet");
  ic0mock_clear_accept_message();
  mockIC.run_test("inspect allowed_greet", canister_inspect_message,
                  "4449444c0000", "", silent_on_trap, my_principal);
  if (!ic0mock_accept_message_called()) {
    std::cout << "\nTest: 'inspect allowed_greet accepts' Failed\n";
    ++extra_failures;
  } else {
    std::cout << "Test: inspect allowed_greet accepts: Passed\n";
  }

  // Any other method must be refused (accept_message not called)
  mockIC.set_msg_method_name("blocked_greet");
  ic0mock_clear_accept_message();
  mockIC.run_test("inspect blocked_greet", canister_inspect_message,
                  "4449444c0000", "", silent_on_trap, my_principal);
  if (ic0mock_accept_message_called()) {
    std::cout << "\nTest: 'inspect blocked_greet refuses' Failed\n";
    ++extra_failures;
  } else {
    std::cout << "Test: inspect blocked_greet refuses: Passed\n";
  }

  // The replica traps on a second accept_message call
  ic0mock_clear_accept_message();
  mockIC.run_trap_test("native_double_accept", native_double_accept,
                       "4449444c0000", silent_on_trap, my_principal);

  // Entry-context guards
  ic0mock_clear_accept_message();
  mockIC.run_trap_test("native_accept_from_update", native_accept_from_update,
                       "4449444c0000", silent_on_trap, my_principal);
  mockIC.run_trap_test("native_method_name_from_update",
                       native_method_name_from_update, "4449444c0000",
                       silent_on_trap, my_principal);

  // The update methods themselves: '()' -> '("Hello!" : text)'
  mockIC.run_test("allowed_greet", allowed_greet, "4449444c0000",
                  "4449444c0001710648656c6c6f21", silent_on_trap, my_principal);
  mockIC.run_test("blocked_greet", blocked_greet, "4449444c0000",
                  "4449444c0001710648656c6c6f21", silent_on_trap, my_principal);

  // returns 1 if any tests failed
  return mockIC.test_summary() + extra_failures;
}
