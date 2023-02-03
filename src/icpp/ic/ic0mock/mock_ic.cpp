// A class to mimic the IC canister runtime, so we can compile,
// develop and debug using a native executable, including
// setting breakpoints in our IDE.

#include "mock_ic.h"
#include "candid_assert.h"
#include "global.h"

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

MockIC::MockIC(const bool &exit_on_fail) {
  m_tests_total = 0;
  m_tests_failed = 0;
  m_B_in.clear();
  m_B_out.clear();
  m_exit_on_fail = exit_on_fail;

  global_mockIC = this;
}

// template <typename Function>
// int MockIC::run_test(const std::string &test_name, Function api,
//                      const std::string &candid_in,
//                      const std::string &candid_out_expected) {

int MockIC::run_test(const std::string &test_name, void (*api_callback)(),
                     const std::string &candid_in,
                     const std::string &candid_out_expected) {
  ++m_tests_total;
  std::cout << "\n----------\n";
  std::cout << "Test: " << test_name << ": \n";

  m_B_in.clear();
  m_B_out.clear();
  m_B_in.store_hex_string(candid_in);

  try {
    // Now call the API we're testing. It will trap if something fails.
    api_callback();

    // Assert the candid_out_expected, which is a string in "hex" format
    if (assert_candid_out(candid_out_expected)) {
      ++m_tests_failed;
      std::cout << "\nTest: '" << test_name << "' Failed\n";
      if (m_exit_on_fail) {
        test_summary();
        exit(1);
      }
      return 1;
    }

    std::cout << "Test: " << test_name << ": Passed\n";
    return 0;
  } catch (const std::exception &e) {
    ++m_tests_failed;
    std::cout << "\nTest: '" << test_name << "' Failed with Exception:\n";
    std::cerr << e.what() << '\n';
    if (m_exit_on_fail) {
      test_summary();
      exit(1);
    }
    return 1;
  }
}

int MockIC::test_summary() {
  std::cout << "\n----------\n";
  std::cout << "Total tests: " << m_tests_total << std::endl;
  std::cout << "- Passed   : " << m_tests_total - m_tests_failed << std::endl;
  std::cout << "- Failed   : " << m_tests_failed << std::endl;

  if (m_tests_failed > 0) {
    return 1;
  }
  return 0;
}

void MockIC::msg_reply_data_append(uintptr_t src, uint32_t size) {
  uint32_t off_now = 0;
  uint8_t *bytes = (uint8_t *)(void *)src;
  for (size_t i = 0; i < size; ++i) {
    // The IC works with a byte stored as (uint8_t). Cast it from that.
    int n = (int)*(bytes + off_now);

    m_B_out.append_byte((std::byte)n);

    off_now += 1;
  }
}

// Assert the serialized candid VecBytes against a string in "hex" format (didc encode)
int MockIC::assert_candid_out(const std::string &candid_out_expected) {
  return CandidAssert::assert_candid(m_B_out, candid_out_expected, true);
}

MockIC::~MockIC() {}