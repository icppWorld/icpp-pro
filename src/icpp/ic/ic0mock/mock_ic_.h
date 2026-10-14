// To be included internally in icpp.

// A class to mimic the IC canister runtime, so we can compile,
// develop and debug using a native executable, including
// setting breakpoints in our IDE.

#pragma once

#include <iostream>
#include <map>
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

  // Mock state for system introspection, subnet & caller info. Sticky too.
  uint64_t get_canister_version() { return m_canister_version; }
  void set_canister_version(uint64_t v) { m_canister_version = v; }
  uint32_t get_canister_status() { return m_canister_status; }
  void set_canister_status(uint32_t v) { m_canister_status = v; }
  // A CanisterQuery is a non-replicated query call unless set otherwise; an
  // update calling a query method makes it replicated on the IC.
  bool get_query_replicated() { return m_query_replicated; }
  void set_query_replicated(bool v) { m_query_replicated = v; }
  uint32_t get_subnet_node_count() { return m_subnet_node_count; }
  void set_subnet_node_count(uint32_t v) { m_subnet_node_count = v; }
  std::vector<uint8_t> get_caller_info_data() { return m_caller_info_data; }
  std::vector<uint8_t> get_caller_info_signer() { return m_caller_info_signer; }
  void set_caller_info(const std::vector<uint8_t> &data,
                       const std::vector<uint8_t> &signer) {
    m_caller_info_data = data;
    m_caller_info_signer = signer;
  }
  void clear_caller_info() {
    m_caller_info_data.clear();
    m_caller_info_signer.clear();
  }

  // Mock state for the canister's environment variables, which on the IC are
  // set with the canister settings. Sticky too; a std::map keeps the names
  // sorted, the order in which ic0_env_var_name reports them.
  std::map<std::string, std::string> get_env_vars() { return m_env_vars; }
  void set_env_var(const std::string &name, const std::string &value) {
    m_env_vars[name] = value;
  }
  void clear_env_vars() { m_env_vars.clear(); }

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
  uint64_t m_canister_version{MOCKIC_INITIAL_CANISTER_VERSION};
  uint32_t m_canister_status{MOCKIC_CANISTER_STATUS_RUNNING};
  bool m_query_replicated{false};
  uint32_t m_subnet_node_count{MOCKIC_SUBNET_NODE_COUNT};
  std::vector<uint8_t> m_caller_info_data;
  std::vector<uint8_t> m_caller_info_signer;
  std::map<std::string, std::string> m_env_vars;
  int m_tests_total;
  int m_tests_failed;
  bool m_exit_on_fail;
  bool m_silent_on_trap;
};