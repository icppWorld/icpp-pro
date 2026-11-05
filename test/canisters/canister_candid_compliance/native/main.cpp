// Runs the Candid specification's test suite natively, and the canister's
// methods for a recursive type through the Mock IC.
// Build it with `icpp build-native`, run it with `icpp run-native`.
//
// Every assert of every suite and every method test must pass, else the
// exit code is 1.

#include "main.h"

#include <iostream>

#include "../src/compliance_runner.h"
#include "../src/my_canister.h"

#include "mock_ic.h"

// The methods list and list_sum, with bytes of didc 0.6.2: `didc encode -d
// src/my_canister.did -t '(List)'`
static int list_tests() {
  MockIC mockIC(false);
  std::string caller{"2vxsx-fae"};
  bool silent_on_trap = true;

  // '(4 : nat)' -> '(opt record { head = 1; tail = opt record { head = 2;
  //                  tail = opt record { head = 3; tail = opt record {
  //                  head = 4; tail = null } } } })'
  const std::string list_4 = "4449444c026e016c02a0d2aca8047c90eddae704000100"
                             "010101020103010400";
  mockIC.run_test("list 4", list, "4449444c00017d04", list_4, silent_on_trap,
                  caller);
  // '(0 : nat)' -> '(null)'
  const std::string list_0 = "4449444c026e016c02a0d2aca8047c90eddae70400010000";
  mockIC.run_test("list 0", list, "4449444c00017d00", list_0, silent_on_trap,
                  caller);
  // A List of 64 nests 129 deep, over the limit of 128
  mockIC.run_trap_test("list 64", list, "4449444c00017d40", silent_on_trap,
                       caller, "Values nest more than 128 deep");

  // List 1, 2, 3, 4 -> '(10 : int)'
  mockIC.run_test("list_sum 4", list_sum, list_4, "4449444c00017c0a",
                  silent_on_trap, caller);
  // '(null)' -> '(0 : int)'
  mockIC.run_test("list_sum 0", list_sum, list_0, "4449444c00017c00",
                  silent_on_trap, caller);
  return mockIC.test_summary();
}

int main() {
  size_t passed = 0;
  size_t total = 0;
  for (const auto &s : compliance_suites()) {
    ComplianceRunner r(s.path);
    s.run(r);
    for (const auto &f : r.failures())
      std::cout << "FAIL " << f << "\n";
    std::cout << s.path << ": " << r.passed() << "/" << r.total()
              << " passed\n";
    passed += r.passed();
    total += r.total();
  }
  std::cout << "Candid compliance: " << passed << "/" << total
            << " asserts passed\n";
  int failed = list_tests();
  return passed == total && failed == 0 ? 0 : 1;
}
