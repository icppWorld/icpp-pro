// A class to mimic the IC canister runtime, so we can compile,
// develop and debug using a native executable, including
// setting breakpoints in our IDE.

#include "candid_assert.h"
#include "candid_serialize.h"
#include "candid_serialize_type_table_registry.h"
#include "candid_type.h"
#include "candid_type_all_includes.h"
#include "current_entry.h"
#include "global.h"
#include "ic0.h"
#include "ic_api.h"
#include "mock_ic_.h"
#include "mock_ic_constants.h"

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
                     const std::string &candid_out_expected,
                     const bool silent_on_trap, const std::string &caller) {
  ++m_tests_total;
  std::cout << "\n----------\n";
  std::cout << "MockIC run_test: " << test_name << ": \n";

  // Always start with a clean type table registry in the registry singleton
  CandidSerializeTypeTableRegistry::get_instance().clear();
  m_B_in.clear();
  m_B_out.clear();
  m_B_in.store_hex_string(candid_in);
  m_caller = CandidTypePrincipal(caller);
  m_canister_self = CandidTypePrincipal(std::string(MOCKIC_CANISTER_SELF));
  start_root_context(test_name);

  m_silent_on_trap = silent_on_trap;
  try {
    // Now call the API we're testing. It will trap if something fails.
    api_callback();

    // Deliver the calls it made. A reply sent from a callback is the reply
    // of the test.
    if (m_auto_drain && !m_events.empty()) {
      drain();
      if (m_contexts[m_root_context].done) {
        m_B_out = m_contexts[m_root_context].reply;
      }
    }

    if (candid_out_expected == "") {
      std::cout << "Test: " << test_name << ": Passed\n";
      return 0;
    }
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

int MockIC::run_test(const std::string &test_name, void (*api_callback)(),
                     const std::string &candid_in,
                     const std::string &candid_out_expected,
                     const bool silent_on_trap, const std::string &caller,
                     std::string *candid_out) {

  int result = run_test(test_name, api_callback, candid_in, candid_out_expected,
                        silent_on_trap, caller);
  if (candid_out) {
    *candid_out = m_B_out.as_hex_string();
  }

  return result;
}

int MockIC::run_trap_test(const std::string &test_name, void (*api_callback)(),
                          const std::string &candid_in,
                          const bool silent_on_trap,
                          const std::string &caller) {
  ++m_tests_total;
  std::cout << "\n----------\n";
  std::cout << "MockIC run_trap_test: " << test_name << ": \n";

  m_B_in.clear();
  m_B_out.clear();
  m_B_in.store_hex_string(candid_in);
  m_caller = CandidTypePrincipal(caller);
  m_canister_self = CandidTypePrincipal(std::string(MOCKIC_CANISTER_SELF));
  start_root_context(test_name);

  m_silent_on_trap = silent_on_trap;
  try {
    // Now call the API we're testing. It must trap.
    api_callback();
    if (m_auto_drain) drain();

    // If it did not trap, give an error message
    ++m_tests_failed;
    std::cout << "\nTest: '" << test_name << "' Failed\n";
    if (m_exit_on_fail) {
      test_summary();
      exit(1);
    }
    return 1;

  } catch (const std::exception &e) {
    std::cout << "Test: " << test_name << ": Passed\n";
    return 0;
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
  uint8_t *bytes = (uint8_t *)(void *)src;
  m_B_out.append_bytes(bytes, size);
}

// Assert the serialized candid VecBytes against a string in "hex" format (didc encode)
int MockIC::assert_candid_out(const std::string &candid_out_expected) {
  return CandidAssert::assert_candid(m_B_out, candid_out_expected, true);
}

// ---------------------------------------------------------------------------
// Inter-canister calls

MockIC::MockResponse MockIC::MockResponse::reply(const std::string &hex,
                                                 __uint128_t accept) {
  MockResponse response;
  response.reply_hex = hex;
  response.accept_cycles = accept;
  return response;
}

MockIC::MockResponse MockIC::MockResponse::reject(uint32_t code,
                                                  const std::string &message) {
  MockResponse response;
  response.is_reply = false;
  response.reject_code = code;
  response.reject_message = message;
  return response;
}

MockIC::MockResponse MockIC::MockResponse::timeout() {
  return reject(6, "IC0506: deadline expired before the response arrived");
}

void MockIC::register_method(const std::string &name, void (*fn)(),
                             MethodKind kind) {
  m_methods[name] = Method{fn, kind};
}

void MockIC::expect_call(const std::string &callee, const std::string &method,
                         const MockResponse &response) {
  m_expected[{callee, method}] = response;
}

// A test starts a new root call context, unless calls are still queued from
// a test that turned auto_drain off: their contexts must stay valid.
void MockIC::start_root_context(const std::string &method) {
  if (m_events.empty()) {
    m_calls.clear();
    m_contexts.clear();
  }
  CallContext context;
  context.caller = m_caller;
  context.method = method;
  context.cycles_available = m_msg_cycles_available;
  m_contexts.push_back(context);
  m_root_context = m_contexts.size() - 1;
  m_current_context = m_root_context;
}

void MockIC::call_new(const std::vector<uint8_t> &callee,
                      const std::string &method, uintptr_t fun, uintptr_t env) {
  m_building = PendingCall{};
  m_building.callee = callee;
  m_building.method = method;
  m_building.fun = fun;
  m_building.env = env;
  m_building.origin = m_current_context;
  CanisterBase *entry = current_entry();
  m_building.origin_composite = entry != nullptr && entry->is_composite();
}

void MockIC::call_data_append(const uint8_t *src, uint32_t size) {
  m_building.args.insert(m_building.args.end(), src, src + size);
}

void MockIC::call_cycles_add(__uint128_t cycles) {
  m_building.cycles += cycles;
}

void MockIC::call_on_cleanup(uintptr_t fun, uintptr_t env) {
  m_building.cleanup_fun = fun;
  m_building.cleanup_env = env;
}

void MockIC::call_with_best_effort_response(uint32_t timeout_seconds) {
  m_building.timeout_seconds = std::min<uint32_t>(timeout_seconds, 300);
}

uint32_t MockIC::call_perform() {
  uint32_t rc = m_next_perform_rc;
  m_next_perform_rc = 0;
  if (rc != 0) return rc;

  // The cycles sent along leave the balance now; what the callee does not
  // accept comes back with the response.
  m_cycles_balance -= m_building.cycles;
  m_calls.push_back(m_building);
  m_contexts[m_current_context].outstanding++;
  Event event;
  event.call = m_calls.size() - 1;
  m_events.push_back(event);
  return 0;
}

// The reply or reject of the current call context answers the call that
// created it.
void MockIC::respond(size_t context, bool is_reply,
                     const std::vector<uint8_t> &reply, uint32_t reject_code,
                     const std::string &reject_message) {
  CallContext &ctx = m_contexts[context];
  ctx.done = true;
  if (ctx.from_call < 0) return; // the root context: the reply of the test
  Event event;
  event.is_response = true;
  event.call = (size_t)ctx.from_call;
  event.is_reply = is_reply;
  event.reply = reply;
  event.reject_code = reject_code;
  event.reject_message = reject_message;
  event.refund = m_msg_cycles_available; // not accepted
  m_events.push_back(event);
}

void MockIC::msg_reply() {
  CallContext &ctx = m_contexts[m_current_context];
  ctx.reply = m_B_out;
  respond(m_current_context, true, m_B_out.vec_uint8_t(), 0, "");
}

void MockIC::msg_reject(const std::string &message) {
  respond(m_current_context, false, {}, 4, message); // CANISTER_REJECT
}

void MockIC::drain() {
  while (!m_events.empty()) {
    Event event = m_events.front();
    m_events.pop_front();
    if (event.is_response) {
      deliver_response(event);
    } else {
      deliver_request(event.call);
    }
  }
}

namespace {
// The Mock IC's state of the message being executed, saved around a nested
// message execution.
struct SavedMessage {
  VecBytes B_in, B_out;
  CandidTypePrincipal caller;
  size_t context;
  __uint128_t cycles_available, cycles_refunded;
  uint32_t reject_code;
  std::string reject_message, method_name;
  uint64_t deadline;
  std::optional<CanisterBase> entry;
  IC_API *ic_api;
};
} // namespace

bool MockIC::management_reply(const std::string &method,
                              std::vector<uint8_t> &reply) {
  ++m_management_counter;
  // Deterministic bytes that differ per call
  auto bytes = [this](size_t n) {
    std::vector<uint8_t> v(n);
    for (size_t i = 0; i < n; ++i)
      v[i] = (uint8_t)(m_management_counter * 31 + i);
    return v;
  };
  CandidArgs out;
  if (method == "raw_rand") {
    out.append(CandidTypeVecNat8{bytes(32)});
  } else if (method == "create_canister") {
    // A new principal, like those of the local network's canisters
    std::vector<uint8_t> id{
        0, 0, 0, 0, 0, 0x10, 0, (uint8_t)m_management_counter, 1, 1};
    CandidTypeRecord result;
    result.append("canister_id", CandidTypePrincipal{id});
    out.append(result);
  } else if (method == "ecdsa_public_key" || method == "schnorr_public_key") {
    CandidTypeRecord result;
    result.append("public_key", CandidTypeVecNat8{bytes(33)});
    result.append("chain_code", CandidTypeVecNat8{bytes(32)});
    out.append(result);
  } else if (method == "sign_with_ecdsa" || method == "sign_with_schnorr") {
    CandidTypeRecord result;
    result.append("signature", CandidTypeVecNat8{bytes(64)});
    out.append(result);
  } else if (method == "vetkd_public_key") {
    CandidTypeRecord result;
    result.append("public_key", CandidTypeVecNat8{bytes(96)});
    out.append(result);
  } else if (method == "vetkd_derive_key") {
    CandidTypeRecord result;
    result.append("encrypted_key", CandidTypeVecNat8{bytes(192)});
    out.append(result);
  } else {
    return false;
  }
  VecBytes B = CandidSerialize(out).get_B();
  CandidSerializeTypeTableRegistry::get_instance().clear();
  reply = B.vec_uint8_t();
  return true;
}

void MockIC::deliver_request(size_t call_index) {
  PendingCall call = m_calls[call_index];
  std::string callee = CandidTypePrincipal(call.callee).get_text();

  // Another canister: the programmed response, if any
  if (callee != m_canister_self.get_text()) {
    Event event;
    event.is_response = true;
    event.call = call_index;
    auto it = m_expected.find({callee, call.method});
    std::vector<uint8_t> management;
    if (it == m_expected.end() && callee == "aaaaa-aa" &&
        management_reply(call.method, management)) {
      // The management canister: what the Mock IC knows of it. The cycles
      // sent along pay for the operation.
      event.is_reply = true;
      event.reply = management;
      event.refund = 0;
    } else if (it == m_expected.end()) {
      event.is_reply = false;
      event.reject_code = 3; // DESTINATION_INVALID
      event.reject_message = "Mock IC: no response for " + callee + "." +
                             call.method + " - set one with expect_call";
      event.refund = call.cycles;
    } else {
      const MockResponse &response = it->second;
      event.is_reply = response.is_reply;
      if (response.is_reply) {
        VecBytes bytes;
        bytes.store_hex_string(response.reply_hex);
        event.reply = bytes.vec_uint8_t();
      }
      event.reject_code = response.reject_code;
      event.reject_message = response.reject_message;
      __uint128_t accepted =
          response.is_reply ? std::min(response.accept_cycles, call.cycles) : 0;
      event.refund = call.cycles - accepted;
    }
    m_events.push_back(event);
    return;
  }

  // The canister itself: run the method, in a new call context.
  // ic_api.yield_fresh_budget() calls __icpp_yield, which only replies.
  if (call.method == "__icpp_yield" && m_methods.count(call.method) == 0) {
    Event event;
    event.is_response = true;
    event.call = call_index;
    event.reply = {'D', 'I', 'D', 'L', 0, 0};
    event.refund = call.cycles;
    m_events.push_back(event);
    return;
  }
  auto method = m_methods.find(call.method);
  if (method == m_methods.end()) {
    Event event;
    event.is_response = true;
    event.call = call_index;
    event.is_reply = false;
    event.reject_code = 5; // CANISTER_ERROR
    event.reject_message = "Mock IC: the canister has no method '" +
                           call.method + "' - register it with register_method";
    event.refund = call.cycles;
    m_events.push_back(event);
    return;
  }
  // The rules of composite queries, as the replica enforces them
  std::string refused;
  if (call.origin_composite && method->second.kind == MethodKind::Update) {
    refused = "Mock IC: a composite query can only call query and composite "
              "query methods, and '" +
              call.method + "' is an update method";
  } else if (!call.origin_composite &&
             method->second.kind == MethodKind::CompositeQuery) {
    refused = "Mock IC: the composite query '" + call.method +
              "' cannot be called in replicated mode";
  }
  if (!refused.empty()) {
    Event event;
    event.is_response = true;
    event.call = call_index;
    event.is_reply = false;
    event.reject_code = 5; // CANISTER_ERROR
    event.reject_message = refused;
    event.refund = call.cycles;
    m_events.push_back(event);
    return;
  }

  SavedMessage saved{m_B_in,
                     m_B_out,
                     m_caller,
                     m_current_context,
                     m_msg_cycles_available,
                     m_msg_cycles_refunded,
                     m_msg_reject_code,
                     m_msg_reject_message,
                     m_msg_method_name,
                     m_msg_deadline,
                     std::nullopt,
                     live_ic_api()};
  if (current_entry() != nullptr) saved.entry = *current_entry();

  CallContext context;
  context.caller = m_canister_self;
  context.method = call.method;
  context.from_call = (int)call_index;
  // A deadline: ns since the epoch, from the timeout of a bounded-wait call
  if (call.timeout_seconds) {
    context.deadline =
        ic0_time() + (uint64_t)*call.timeout_seconds * 1'000'000'000;
  }
  m_contexts.push_back(context);
  m_current_context = m_contexts.size() - 1;
  m_B_in.store(call.args.data(), (uint32_t)call.args.size());
  m_B_out.clear();
  m_caller = m_canister_self;
  m_msg_cycles_available = call.cycles;
  m_msg_reject_code = 0;
  m_msg_reject_message.clear();
  m_msg_method_name = call.method;
  m_msg_deadline = m_contexts[m_current_context].deadline;

  size_t index = m_current_context;
  try {
    method->second.fn();
  } catch (const std::exception &e) {
    std::cout << "Mock IC: '" << call.method << "' trapped: " << e.what()
              << "\n";
    if (!m_contexts[index].done) {
      respond(index, false, {}, 5, std::string("trapped: ") + e.what());
    }
  }
  CallContext &ctx = m_contexts[index];
  ctx.cycles_available = m_msg_cycles_available;
  if (!ctx.done && ctx.outstanding == 0) {
    respond(index, false, {}, 5, "the canister did not reply to the call");
  }

  m_B_in = saved.B_in;
  m_B_out = saved.B_out;
  m_caller = saved.caller;
  m_current_context = saved.context;
  m_msg_cycles_available = saved.cycles_available;
  m_msg_cycles_refunded = saved.cycles_refunded;
  m_msg_reject_code = saved.reject_code;
  m_msg_reject_message = saved.reject_message;
  m_msg_method_name = saved.method_name;
  m_msg_deadline = saved.deadline;
  if (saved.entry) {
    set_current_entry(*saved.entry);
  } else {
    clear_current_entry();
  }
  set_live_ic_api(saved.ic_api);
}

void MockIC::deliver_response(const Event &event) {
  PendingCall call = m_calls[event.call];
  size_t origin = call.origin;
  m_contexts[origin].outstanding--;
  m_cycles_balance += event.refund;

  // A oneway call has no callback
  constexpr uintptr_t ONEWAY_FUN = 0xFFFFFFFF;
  if (call.fun == 0 || call.fun == ONEWAY_FUN) return;

  SavedMessage saved{m_B_in,
                     m_B_out,
                     m_caller,
                     m_current_context,
                     m_msg_cycles_available,
                     m_msg_cycles_refunded,
                     m_msg_reject_code,
                     m_msg_reject_message,
                     m_msg_method_name,
                     m_msg_deadline,
                     std::nullopt,
                     live_ic_api()};
  if (current_entry() != nullptr) saved.entry = *current_entry();

  CallContext &ctx = m_contexts[origin];
  m_current_context = origin;
  m_B_in.clear();
  if (event.is_reply) {
    m_B_in.store(event.reply.data(), (uint32_t)event.reply.size());
  }
  m_B_out.clear();
  m_caller = ctx.caller;
  m_msg_cycles_available = ctx.cycles_available;
  m_msg_cycles_refunded = event.refund;
  m_msg_reject_code = event.is_reply ? 0 : event.reject_code;
  m_msg_reject_message = event.is_reply ? "" : event.reject_message;
  m_msg_method_name = ctx.method;
  m_msg_deadline = ctx.deadline; // of the call this context answers

  try {
    reinterpret_cast<void (*)(uintptr_t)>(call.fun)(call.env);
  } catch (const std::exception &e) {
    std::cout << "Mock IC: the callback of '" << call.method
              << "' trapped: " << e.what() << "\n";
    if (call.cleanup_fun != 0) {
      try {
        reinterpret_cast<void (*)(uintptr_t)>(call.cleanup_fun)(
            call.cleanup_env);
      } catch (const std::exception &e2) {
        std::cout << "Mock IC: the cleanup of '" << call.method
                  << "' trapped: " << e2.what() << "\n";
      }
    }
  }
  CallContext &after = m_contexts[origin];
  after.cycles_available = m_msg_cycles_available;
  if (!after.done && after.outstanding == 0 && after.from_call >= 0) {
    respond(origin, false, {}, 5, "the canister did not reply to the call");
  }

  m_B_in = saved.B_in;
  m_B_out = saved.B_out;
  m_caller = saved.caller;
  m_current_context = saved.context;
  m_msg_cycles_available = saved.cycles_available;
  m_msg_cycles_refunded = saved.cycles_refunded;
  m_msg_reject_code = saved.reject_code;
  m_msg_reject_message = saved.reject_message;
  m_msg_method_name = saved.method_name;
  m_msg_deadline = saved.deadline;
  if (saved.entry) {
    set_current_entry(*saved.entry);
  } else {
    clear_current_entry();
  }
  set_live_ic_api(saved.ic_api);
}

MockIC::~MockIC() {}