// A class to mimic the IC canister runtime, so we can compile,
// develop and debug using a native executable, including
// setting breakpoints in our IDE.

#pragma once

#include "ic_api.h"
#include <iostream>
#include <vector>

class MockIC {
public:
  // Constructors
  explicit MockIC(const bool &exit_on_fail);

  // Destructor
  ~MockIC();

  std::vector<std::byte> vec_in() { return m_B_in.vec(); };
  std::vector<std::byte> vec_out() { return m_B_out.vec(); };

  VecBytes get_msg_in() { return m_B_in; };
  VecBytes get_msg_out() { return m_B_out; };

  void msg_reply_data_append(uintptr_t src, uint32_t size);

  int assert_candid_out(const std::string &candid_out_expected);

  // template <typename Function>
  // int run_test(const std::string &test_name, Function api,
  //              const std::string &candid_in, const std::string &candid_out_expected);

  int run_test(const std::string &test_name, void (*api_callback)(),
               const std::string &candid_in,
               const std::string &candid_out_expected);

  int test_summary();

private:
  VecBytes m_B_in;
  VecBytes m_B_out;
  int m_tests_total;
  int m_tests_failed;
  bool m_exit_on_fail;
};