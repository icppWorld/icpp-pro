// A class to mimic the IC canister runtime, so we can compile,
// develop and debug using a native executable, including
// setting breakpoints in our IDE.

#include "mock_ic.h"
#include "candid_assert.h"
#include "global.h"
#include "pro.h"

#include <cstddef>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include <iterator>
#include <limits>
#include <numeric>
#include <sstream>

MockIC::MockIC(const bool &exit_on_fail) { Pro().exit_if_not_pro(); }

int MockIC::run_test(const std::string &test_name, void (*api_callback)(),
                     const std::string &candid_in,
                     const std::string &candid_out_expected,
                     const bool silent_on_trap, const std::string &caller) {
  return 1;
}

int MockIC::run_trap_test(const std::string &test_name, void (*api_callback)(),
                          const std::string &candid_in,
                          const bool silent_on_trap,
                          const std::string &caller) {
  return 1;
}

int MockIC::test_summary() { return 1; }

void MockIC::msg_reply_data_append(uintptr_t src, uint32_t size) {}

// Assert the serialized candid VecBytes against a string in "hex" format (didc encode)
int MockIC::assert_candid_out(const std::string &candid_out_expected) {
  return 1;
}

MockIC::~MockIC() {}