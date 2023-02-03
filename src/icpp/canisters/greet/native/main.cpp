// C++ example canister

// Main entry point for a native debug executable.
// Build it with: `icpp build-native` from the parent folder where 'icpp.toml' resides

#include "main.h"

#include <iostream>

#include "../src/greet.h"

// The Mock IC
#include "global.h"
#include "mock_ic.h"

int main() {
  MockIC mockIC(true);

  // '()' -> '("hello!")'
  mockIC.run_test("greet_0", greet_0, "4449444c0000",
                  "4449444c0001710668656c6c6f21");

  // '()' -> '(2023)'
  mockIC.run_test("greet_1", greet_1, "4449444c0000", "4449444c00017ce70f");

  // '("C++ Developer")' -> '("hello C++ Developer!")'
  mockIC.run_test("greet_2", greet_2,
                  "4449444c0001710d432b2b20446576656c6f706572",
                  "4449444c0001711468656c6c6f20432b2b20446576656c6f70657221");

  // '(record { "icpp version" = 1 : int; OS = "Linux" : text })' ->
  // '(record { "icpp Release Details" = "Version = 1 & Operating System = Linux" : text; "release year" = 2023 : int })'
  mockIC.run_test(
      "greet_3", greet_3,
      "4449444c016c02a48a0171d2fae7d70d7c0100054c696e757801",
      "4449444c016c02e3aa95930271b685be880f7c01002656657273696f6e203d20312026204f7065726174696e672053797374656d203d204c696e7578e70f");

  // '(record { 6 = 42 : int; 9 = 43 : int }, record { 7 = 44 : int; 10 = 45 : int })' ->
  // '("Hello!", "Your secret numbers are:", 42 : int, 43 : int, 44 : int, 45 : int)'
  mockIC.run_test(
      "greet_4", greet_4, "4449444c026c02067c097c6c02077c0a7c0200012a2b2c2d",
      "4449444c000671717c7c7c7c0648656c6c6f2118596f757220736563726574206e756d62657273206172653a2a2b2c2d");

  // returns 1 if any tests failed
  return mockIC.test_summary();
}