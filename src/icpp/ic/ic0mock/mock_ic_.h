// To be included internally in icpp.

// A class to mimic the IC canister runtime, so we can compile,
// develop and debug using a native executable, including
// setting breakpoints in our IDE.

#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "mock_ic_constants.h"

class MockIC {
public:
  // Constructors
  explicit MockIC(const bool &exit_on_fail);

  // Destructor
  ~MockIC();

  std::vector<std::byte> vec_in() { return m_B_in.vec(); }
  std::vector<std::byte> vec_out() { return m_B_out.vec(); }
  CandidTypePrincipal get_caller() { return m_caller; }
  CandidTypePrincipal get_canister_self() { return m_canister_self; }
  __uint128_t get_canister_self_cycle_balance() { return m_cycles_balance; }

  VecBytes get_msg_in() { return m_B_in; }
  VecBytes get_msg_out() { return m_B_out; }

  // Mock state for 128-bit cycles, inspect message & certified data.
  // Sticky, like ic0mock_set_time_override: NOT reset by run_test /
  // run_trap_test. Tests set what a scenario needs and clear it on exit.
  __uint128_t get_msg_cycles_available() { return m_msg_cycles_available; }
  void set_msg_cycles_available(__uint128_t v) { m_msg_cycles_available = v; }
  __uint128_t get_msg_cycles_refunded() { return m_msg_cycles_refunded; }
  void set_msg_cycles_refunded(__uint128_t v) { m_msg_cycles_refunded = v; }
  void add_cycles_balance(__uint128_t v) { m_cycles_balance += v; }
  void sub_cycles_balance(__uint128_t v) { m_cycles_balance -= v; }
  std::string get_msg_method_name() { return m_msg_method_name; }
  void set_msg_method_name(const std::string &name) {
    m_msg_method_name = name;
  }
  bool get_data_certificate_present() { return m_data_certificate_present; }
  std::vector<uint8_t> get_data_certificate() { return m_data_certificate; }
  void set_data_certificate(const std::vector<uint8_t> &cert) {
    m_data_certificate = cert;
    m_data_certificate_present = true;
  }
  void clear_data_certificate() {
    m_data_certificate.clear();
    m_data_certificate_present = false;
  }
  // What the canister stored via ic0.certified_data_set - so a native test
  // can assert it.
  std::vector<uint8_t> get_certified_data() { return m_certified_data; }
  void set_certified_data(const std::vector<uint8_t> &data) {
    m_certified_data = data;
  }

  void msg_reply_data_append(uintptr_t src, uint32_t size);

  int assert_candid_out(const std::string &candid_out_expected);

  int run_test(const std::string &test_name, void (*api_callback)(),
               const std::string &candid_in,
               const std::string &candid_out_expected,
               const bool silent_on_trap = false,
               const std::string &caller = "");

  int run_test(const std::string &test_name, void (*api_callback)(),
               const std::string &candid_in,
               const std::string &candid_out_expected,
               const bool silent_on_trap, const std::string &caller,
               std::string *candid_out);

  int run_trap_test(const std::string &test_name, void (*api_callback)(),
                    const std::string &candid_in,
                    const bool silent_on_trap = true,
                    const std::string &caller = "");

  int test_summary();

  bool get_silent_on_trap() { return m_silent_on_trap; }

private:
  VecBytes m_B_in;
  VecBytes m_B_out;
  CandidTypePrincipal m_caller;
  CandidTypePrincipal m_canister_self;
  __uint128_t m_cycles_balance{MOCKIC_INITIAL_CYCLES_BALANCE};
  __uint128_t m_msg_cycles_available{0};
  __uint128_t m_msg_cycles_refunded{0};
  std::string m_msg_method_name;
  std::vector<uint8_t> m_data_certificate;
  bool m_data_certificate_present{false};
  std::vector<uint8_t> m_certified_data;
  int m_tests_total;
  int m_tests_failed;
  bool m_exit_on_fail;
  bool m_silent_on_trap;
};