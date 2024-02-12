// Main entry point for a native debug executable.
// Build it with: `icpp build-native` from the parent folder where 'icpp.toml' resides

#include "main.h"

#include <iostream>

#include "../src/greet.h"

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
  // '()' -> '("hello!")'
  mockIC.run_test("query_greet", query_greet, "4449444c0000",
                  "4449444c0001710668656c6c6f21", silent_on_trap, my_principal);

  // '()' -> '("hello 32!")'
  mockIC.run_test("query_greet32", query_greet32, "4449444c0000",
                  "4449444c0001710968656c6c6f20333221", silent_on_trap,
                  my_principal);

  // '()' -> '("hello 64!")'
  mockIC.run_test("query_greet64", query_greet64, "4449444c0000",
                  "4449444c0001710968656c6c6f20363421", silent_on_trap,
                  my_principal);

  // -----------------------------------------------------------------------------
  // '()' -> '("hello!")'
  mockIC.run_test("update_greet", update_greet, "4449444c0000",
                  "4449444c0001710668656c6c6f21", silent_on_trap, my_principal);

  // '()' -> '("hello 32!")'
  mockIC.run_test("update_greet32", update_greet32, "4449444c0000",
                  "4449444c0001710968656c6c6f20333221", silent_on_trap,
                  my_principal);

  // '()' -> '("hello 64!")'
  mockIC.run_test("update_greet64", update_greet64, "4449444c0000",
                  "4449444c0001710968656c6c6f20363421", silent_on_trap,
                  my_principal);

  // returns 1 if any tests failed
  return mockIC.test_summary();
}