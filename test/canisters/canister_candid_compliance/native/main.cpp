// Runs the Candid specification's test suite natively.
// Build it with `icpp build-native`, run it with `icpp run-native`.
//
// Every assert of every suite must pass: the exit code is the number of
// failed asserts.

#include "main.h"

#include <iostream>

#include "../src/compliance_runner.h"

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
  return passed == total ? 0 : 1;
}
