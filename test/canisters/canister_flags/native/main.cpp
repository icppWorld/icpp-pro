// Main entry point for a native debug executable.
// Build it with: `icpp build-native` from the parent folder where 'icpp.toml' resides

#include "main.h"

#include <iostream>

#include "../src/my_canister.h"

// The Mock IC
#include "mock_ic.h"

int main() {
  MockIC mockIC(true);

  // -----------------------------------------------------------------------------
  // Configs for the tests:

  // The pretend principals of the caller
  std::string my_principal{
      "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae"};
  std::string anonymous_principal{"2vxsx-fae"};

  bool silent_on_trap = true;

  // -----------------------------------------------------------------------------
  // '("{\"name\": \"AJ\"}")' -> '("{\"greet\":\"Hello AJ!\"}")'
  mockIC.run_test("greet_json", greet_json,
                  "4449444c0001710e7b226e616d65223a2022414a227d",
                  "4449444c000171157b226772656574223a2248656c6c6f20414a21227d",
                  silent_on_trap, my_principal);

  // returns 1 if any tests failed
  return mockIC.test_summary();
}