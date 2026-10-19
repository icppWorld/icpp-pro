// To be included internally in icpp.

// A class to mimic the IC canister runtime, so we can compile,
// develop and debug using a native executable, including
// setting breakpoints in our IDE.

#pragma once

#include <cstdint>
#include <deque>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <utility>
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
  // Stopping or stopped: calls to the canister itself are rejected, like on
  // the IC; responses to calls in flight are still delivered.
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

  // ic0.performance_counter returns steady-clock nanoseconds natively. A test
  // of code that sizes its work by instructions can feed its own counter,
  // e.g. one it advances per step: fn(counter_type). Sticky too.
  const std::function<uint64_t(uint32_t)> &get_performance_counter() const {
    return m_performance_counter;
  }
  void set_performance_counter(std::function<uint64_t(uint32_t)> fn) {
    m_performance_counter = std::move(fn);
  }
  void clear_performance_counter() { m_performance_counter = nullptr; }

  // -------------------------------------------------------------------------
  // Inter-canister calls (ic_async.h).
  //
  // A performed call is queued. After the tested entry point returns,
  // run_test & run_trap_test deliver the queued calls one by one, like the
  // IC: a call to the canister itself runs the method registered under its
  // name, a call to another canister gets the response set with expect_call,
  // and the response runs the caller's callback. Unlike the IC, the Mock IC
  // does not roll back the state changes of a trapped message.
  // A trap after a co_await counts as a trap, also when another coroutine of
  // the same call replies later: it fails run_test, and passes run_trap_test.

  // The response of another canister to a call.
  struct MockResponse {
    bool is_reply{true};
    std::string reply_hex;        // the Candid reply, as didc encode hex
    __uint128_t accept_cycles{0}; // accepted of the cycles sent along
    uint32_t reject_code{0};      // IC_RejectCode
    std::string reject_message;
    static MockResponse reply(const std::string &hex, __uint128_t accept = 0);
    static MockResponse reject(uint32_t code, const std::string &message);
    // A best-effort call whose deadline expired: rejected with SYS_UNKNOWN
    static MockResponse timeout();
  };

  // The kind of a registered method: a composite query can only call query
  // and composite query methods, and a composite query can't be called from
  // an update, a callback or a timer (replicated mode).
  enum class MethodKind { Update, Query, CompositeQuery };
  // A method of this canister, so a call to the canister itself can run it.
  void register_method(const std::string &name, void (*fn)(),
                       MethodKind kind = MethodKind::Update);
  // The response of canister `callee` (principal text) to `method`. Sticky.
  void expect_call(const std::string &callee, const std::string &method,
                   const MockResponse &response);
  void clear_expected_calls() { m_expected.clear(); }
  // The result of the next ic0.call_perform (0: performed).
  void set_next_call_perform_rc(uint32_t rc) { m_next_perform_rc = rc; }
  // false: keep calls queued after an entry point, until drain().
  void set_auto_drain(bool auto_drain) { m_auto_drain = auto_drain; }
  // Delivers the queued calls & responses until none are left.
  void drain();
  // Delivers the next queued call or response. False when none was queued.
  bool drain_one();
  size_t pending_calls() { return m_events.size(); }

  // Calls that interleave: start_call runs the entry point and returns a
  // handle, without delivering the calls it made, nor asserting a reply.
  // After drain() or drain_one(), the handle tells how the call ended:
  //   mockIC.set_auto_drain(false);
  //   MockIC::CallHandle a = mockIC.start_call("a", my_method, IN_A);
  //   MockIC::CallHandle b = mockIC.start_call("b", my_method, IN_B);
  //   mockIC.drain();
  //   mockIC.reply_hex(a); mockIC.reply_hex(b);
  // A handle stays valid until a run_test or run_trap_test starts with no
  // calls queued: that forgets all calls, and using an older handle traps.
  // Unlike the IC, the Mock IC runs a call that a test starts on a stopping
  // canister.
  struct CallHandle {
    uint64_t generation;
    size_t context;
  };
  CallHandle start_call(const std::string &name, void (*api_callback)(),
                        const std::string &candid_in,
                        const std::string &caller = "");
  // The call replied or rejected
  bool call_done(CallHandle h) { return context(h).done; }
  // It rejected, or trapped
  bool was_rejected(CallHandle h) { return context(h).rejected; }
  // The Candid reply, as hex
  std::string reply_hex(CallHandle h) {
    return context(h).reply.as_hex_string();
  }
  std::string reject_message(CallHandle h) { return context(h).reject_message; }

  // For the ic0 mock functions
  void call_new(const std::vector<uint8_t> &callee, const std::string &method,
                uintptr_t fun, uintptr_t env);
  void call_data_append(const uint8_t *src, uint32_t size);
  void call_cycles_add(__uint128_t cycles);
  void call_on_cleanup(uintptr_t fun, uintptr_t env);
  void call_with_best_effort_response(uint32_t timeout_seconds);
  uint32_t call_perform();
  void msg_reply();
  void msg_reject(const std::string &message);
  uint32_t get_msg_reject_code() { return m_msg_reject_code; }
  std::string get_msg_reject_message() { return m_msg_reject_message; }
  uint64_t get_msg_deadline() { return m_msg_deadline; }

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
  std::function<uint64_t(uint32_t)> m_performance_counter;
  // Inter-canister calls
  struct PendingCall {
    std::vector<uint8_t> callee;
    std::string method;
    std::vector<uint8_t> args;
    __uint128_t cycles{0};
    uintptr_t fun{0};
    uintptr_t env{0};
    uintptr_t cleanup_fun{0};
    uintptr_t cleanup_env{0};
    std::optional<uint32_t> timeout_seconds;
    size_t origin{0};             // the call context that made the call
    bool origin_composite{false}; // made from a composite query
  };
  // A call context: the root one of a test, or one per delivered call
  struct CallContext {
    CandidTypePrincipal caller;
    std::string method;
    bool done{false};
    VecBytes reply;
    bool rejected{false};
    std::string reject_message;
    bool trapped{false};      // a callback of this call trapped
    std::string trap_message; // of the first trap
    int outstanding{0};       // performed calls without a response yet
    int from_call{-1};        // the call it answers
    __uint128_t cycles_available{0};
    uint64_t deadline{0}; // of the call it answers: 0 unless bounded-wait
  };
  struct Event {
    bool is_response{false};
    size_t call{0};
    bool is_reply{true};
    std::vector<uint8_t> reply;
    uint32_t reject_code{0};
    std::string reject_message;
    __uint128_t refund{0};
  };
  // Sets up the message of a test, and its root call context. `reset`: when
  // no calls are queued, forget the earlier calls.
  void start_message(const std::string &test_name, const std::string &candid_in,
                     const std::string &caller, bool reset);
  void start_root_context(const std::string &method, bool reset);
  // The call context of a handle. Traps when the handle is stale.
  CallContext &context(CallHandle h);
  uint64_t m_generation{0}; // incremented when the calls are forgotten
  void deliver_request(size_t call);
  // The management canister's reply to `method`, if the Mock IC knows it:
  // deterministic bytes of the right length. False when it does not.
  bool management_reply(const std::string &method, std::vector<uint8_t> &reply);
  uint64_t m_management_counter{0};
  void deliver_response(const Event &event);
  void respond(size_t context, bool is_reply, const std::vector<uint8_t> &reply,
               uint32_t reject_code, const std::string &reject_message);
  std::vector<PendingCall> m_calls;
  std::vector<CallContext> m_contexts;
  std::deque<Event> m_events;
  size_t m_current_context{0};
  size_t m_root_context{0};
  PendingCall m_building;
  struct Method {
    void (*fn)();
    MethodKind kind;
  };
  std::map<std::string, Method> m_methods;
  std::map<std::pair<std::string, std::string>, MockResponse> m_expected;
  uint32_t m_next_perform_rc{0};
  bool m_auto_drain{true};
  uint32_t m_msg_reject_code{0};
  std::string m_msg_reject_message;
  uint64_t m_msg_deadline{0};

  int m_tests_total;
  int m_tests_failed;
  bool m_exit_on_fail;
  bool m_silent_on_trap;
};