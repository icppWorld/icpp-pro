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
  // '()' -> '("Hello World")'
  mockIC.run_test("greet", greet, "4449444c0000",
                  "4449444c0001710b48656c6c6f20576f726c64", silent_on_trap,
                  my_principal);

  // returns 1 if any tests failed
  return mockIC.test_summary();
}