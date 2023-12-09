// Main entry point for a native debug executable.
// Build it with: `icpp build-native` from the parent folder where 'icpp.toml' resides

#include "main.h"

#include <iostream>

#include "../src/my_canister.h"

// The Mock IC
#include "icpp_hooks.h"
#include "mock_ic.h"

int main() {
  bool exit_on_fail = true;
  MockIC mockIC(exit_on_fail);

  // The pretend principals of the caller
  std::string my_principal{
      "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae"};
  std::string anonymous_principal{"2vxsx-fae"};

  bool silent_on_trap = true;

  //----------------------------------------------------------------------------------
  // Run all roundtrip tests

  // ------------------------------------------------------------------------
  // Record with Vec Record field (headers of http_request)
  // '(record { headers = vec { record {name = "H1N" : text; value = "H1V"}; record {name = "H2N" : text; value = "H2V"}; record {name = "H3N" : text; value = "H3V"}; } })'
  // -> same
  // Use compact type table of didc
  mockIC.run_test(
      "roundtrip_record_vec_record", roundtrip_record_vec_record,
      "4449444c036c01c6a4a19806016d026c02f1fee18d0371cbe4fdc70471010003034831560348314e034832560348324e034833560348334e",
      "4449444c096c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c01c6a4a1980604010803034831560348314e034832560348324e034833560348334e",
      silent_on_trap, my_principal);
  // Use larger type table of icpp
  mockIC.run_test(
      "roundtrip_record_vec_record 2", roundtrip_record_vec_record,
      "4449444c096c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c01c6a4a1980604010803034831560348314e034832560348324e034833560348334e",
      "4449444c096c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c01c6a4a1980604010803034831560348314e034832560348324e034833560348334e",
      silent_on_trap, my_principal);

  // returns 1 if any tests failed
  return mockIC.test_summary();
}
