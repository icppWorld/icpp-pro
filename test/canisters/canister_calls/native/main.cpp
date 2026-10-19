// Main entry point for a native debug executable.
// Build it with: `icpp build-native` from the parent folder where 'icpp.toml'
// resides.
//
// The Mock IC delivers the calls a method makes after it returns: a call to
// the canister itself (MOCKIC_CANISTER_SELF) runs the registered method, a
// call to another canister gets the response set with expect_call.

#include "main.h"

#include <iostream>
#include <string>

#include "../src/my_canister.h"

// The Mock IC
// clang-format off
#include "ic_api.h"
#include "icpp_hooks.h"
#include "mock_ic.h"
#include "ic0.h"
// clang-format on

// The weak default canister_pre_upgrade of ic_async.cpp
void icpp_default_pre_upgrade() WASM_SYMBOL_EXPORTED("canister_pre_upgrade");

namespace {
// Another canister, with programmed responses
const std::string CANISTER_B{"ryjl3-tyaaa-aaaaa-aaaba-cai"};

// Candid hex of the arguments: (principal, ...) with the canister itself or
// canister B
const std::string GREET_SELF{
    "4449444c00026871010a0000000000c0115a010103432b2b"}; // (self, "C++")
const std::string GREET_B{
    "4449444c00026871010a0000000000000002010103432b2b"}; // (B, "C++")
const std::string P_SELF{"4449444c000168010a0000000000c0115a0101"}; // (self)
const std::string P_B{"4449444c000168010a00000000000000020101"};    // (B)
const std::string CYCLES_SELF{
    "4449444c00026878010a0000000000c0115a010140420f0000000000"}; // (self, 1M)
const std::string BOUNDED_B{
    "4449444c0003687979010a000000000000000201010100000003000000"}; // (B,1,3)
const std::string N3{"4449444c00017903000000"};                    // (3)
const std::string N2{"4449444c00017902000000"};                    // (2)
const std::string N100{"4449444c00017964000000"};                  // (100)
const std::string EMPTY{"4449444c0000"};                           // ()
const std::string P_SELF_SELF{
    "4449444c00026868010a0000000000c0115a0101010a0000000000c0115a0101"};
// (self, "info_cq", self) and (self, "info", self)
const std::string TIMER_INFO_CQ{
    "4449444c0003687168010a0000000000c0115a010107696e"
    "666f5f6371010a0000000000c0115a0101"};
const std::string TIMER_INFO{
    "4449444c0003687168010a0000000000c0115a010104696e66"
    "6f010a0000000000c0115a0101"};

const std::string OUT_HELLO{"4449444c0001710b48656c6c6f2c20432b2b21"};
const std::string OUT_HI_B{"4449444c0001710948692066726f6d2042"};
const std::string OUT_CYCLES{
    "4449444c0002787820a107000000000020a1070000000000"}; // (500_000, 500_000)

// The text of a Candid reply, from hex
std::string text_of(const std::string &hex) {
  VecBytes out;
  out.store_hex_string(hex);
  std::string text;
  CandidArgs args;
  args.append(CandidTypeText{&text});
  CandidDeserialize(out, args);
  return text;
}

// Runs a method that replies with text, and checks the text starts with
// `prefix`.
int run_text_test(MockIC &mockIC, const std::string &name, void (*fn)(),
                  const std::string &in, const std::string &prefix,
                  const std::string &caller) {
  std::string out_hex;
  mockIC.run_test(name, fn, in, "", true, caller, &out_hex);
  std::string text = text_of(out_hex);
  if (text.rfind(prefix, 0) != 0) {
    std::cout << "Test '" << name << "' Failed: '" << text
              << "' does not start with '" << prefix << "'\n";
    exit(1);
  }
  std::cout << "Test '" << name << "': reply '" << text << "' - Passed\n";
  return 0;
}

// Candid hex of the arguments of call_chain: (vec principal, text)
std::string chain_args(const std::vector<std::string> &hops,
                       const std::string &name) {
  CandidArgs args;
  args.append(CandidTypeVecPrincipal{hops});
  args.append(CandidTypeText{name});
  VecBytes bytes = CandidSerialize(args).get_B();
  return bytes.as_hex_string();
}

// The LCG of compute(), iterated n times from seed
uint64_t lcg(uint64_t x, uint64_t n) {
  for (uint64_t i = 0; i < n; ++i)
    x = 6364136223846793005ULL * x + 1442695040888963407ULL;
  return x;
}

// Candid hex of the arguments of compute
std::string compute_args(uint32_t job, uint64_t seed, uint64_t n, bool reload) {
  CandidArgs args;
  args.append(CandidTypeNat32{job});
  args.append(CandidTypeNat64{seed});
  args.append(CandidTypeNat64{n});
  args.append(CandidTypeNat64{1}); // chunk_instructions: one block a chunk
  args.append(CandidTypeBool{reload});
  VecBytes bytes = CandidSerialize(args).get_B();
  return bytes.as_hex_string();
}

// Decodes a reply of the given Candid args from hex
void decode(const std::string &hex, CandidArgs &args) {
  VecBytes bytes;
  bytes.store_hex_string(hex);
  CandidDeserialize(bytes, args);
}

void check(bool ok, const std::string &what) {
  if (!ok) {
    std::cout << "Test '" << what << "' Failed\n";
    exit(1);
  }
  std::cout << "Test '" << what << "' - Passed\n";
}

// The saved state of a job, read with get_job
uint64_t job_x(MockIC &mockIC, uint32_t job, const std::string &caller) {
  CandidArgs in;
  in.append(CandidTypeNat32{job});
  VecBytes in_bytes = CandidSerialize(in).get_B();
  std::string out_hex;
  mockIC.run_test("get_job " + std::to_string(job), get_job,
                  in_bytes.as_hex_string(), "", true, caller, &out_hex);
  uint64_t x{0}, i{0};
  CandidArgs out;
  out.append(CandidTypeNat64{&x});
  out.append(CandidTypeNat64{&i});
  decode(out_hex, out);
  return x;
}

// Starts 3 jobs before any of them finished, and lets them interleave.
// Returns how many jobs got the right result.
int run_interleaved_jobs(MockIC &mockIC, bool reload,
                         const std::string &caller) {
  const uint64_t n = 200'000; // 4 chunks of LCG_BLOCK steps
  mockIC.run_test("clear_run_log", clear_run_log, EMPTY, "4449444c0000", true,
                  caller);
  mockIC.set_auto_drain(false);
  for (uint32_t job = 1; job <= 3; ++job) {
    mockIC.run_test("compute job " + std::to_string(job), compute,
                    compute_args(job, 1000 + job, n, reload), "", true, caller);
  }
  mockIC.drain();
  mockIC.set_auto_drain(true);

  std::string log_hex;
  mockIC.run_test("get_run_log", get_run_log, EMPTY, "", true, caller,
                  &log_hex);
  std::vector<uint32_t> log;
  CandidArgs out;
  out.append(CandidTypeVecNat32{&log});
  decode(log_hex, out);
  // The chunks of the jobs took turns: 1, 2, 3, 1, 2, 3, ...
  check(log.size() >= 4 && log[0] == 1 && log[1] == 2 && log[2] == 3 &&
            log[3] == 1,
        "the chunks of the jobs interleave");

  int right{0};
  for (uint32_t job = 1; job <= 3; ++job) {
    if (job_x(mockIC, job, caller) == lcg(1000 + job, n)) ++right;
  }
  return right;
}

// (before, after, destroyed, calls_in_flight, held_done), read with get_state
struct State {
  uint64_t before{0}, after{0}, destroyed{0};
  uint32_t in_flight{0};
  bool held_done{false};
};
State read_state(MockIC &mockIC, const std::string &caller) {
  std::string out_hex;
  mockIC.run_test("get_state", get_state, "4449444c0000", "", true, caller,
                  &out_hex);
  State state;
  CandidArgs out;
  out.append(CandidTypeNat64{&state.before});
  out.append(CandidTypeNat64{&state.after});
  out.append(CandidTypeNat64{&state.destroyed});
  out.append(CandidTypeNat32{&state.in_flight});
  out.append(CandidTypeBool{&state.held_done});
  decode(out_hex, out);
  return state;
}

// Candid hex of the arguments of call_all: (vec principal, vec text)
std::string calls_args(const std::vector<std::string> &callees,
                       const std::vector<std::string> &methods) {
  CandidArgs args;
  args.append(CandidTypeVecPrincipal{callees});
  args.append(CandidTypeVecText{methods});
  VecBytes bytes = CandidSerialize(args).get_B();
  return bytes.as_hex_string();
}

// Runs call_all, and checks its vec text reply
void check_call_all(MockIC &mockIC, const std::string &name,
                    const std::string &in,
                    const std::vector<std::string> &expected,
                    const std::string &caller) {
  std::string out_hex;
  mockIC.run_test(name, call_all, in, "", true, caller, &out_hex);
  std::vector<std::string> out;
  CandidArgs args;
  args.append(CandidTypeVecText{&out});
  decode(out_hex, args);
  check(out == expected, name + ": the results, in the order of the calls");
}

// '()' -> trap while calls are in flight (the IC_API constructor path)
void native_pre_upgrade_with_ic_api() {
  IC_API ic_api(CanisterPreUpgrade{"native_pre_upgrade_with_ic_api"}, false);
}
} // namespace

int main() {
  bool exit_on_fail = true;
  MockIC mockIC(exit_on_fail);

  std::string my_principal{
      "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae"};
  bool silent_on_trap = true;

  // The methods a call to the canister itself can run
  mockIC.register_method("greet", greet);
  mockIC.register_method("reject_me", reject_me);
  mockIC.register_method("trap_me", trap_me);
  mockIC.register_method("accept_half", accept_half);
  mockIC.register_method("slow", slow);
  mockIC.register_method("hold", hold);
  mockIC.register_method("call_chain", call_chain);
  mockIC.register_method("greet_q", greet_q, MockIC::MethodKind::Query);
  mockIC.register_method("cq_name", cq_name,
                         MockIC::MethodKind::CompositeQuery);
  mockIC.register_method("info", info);
  mockIC.register_method("info_cq", info_cq,
                         MockIC::MethodKind::CompositeQuery);

  // A call to the canister itself, and to canister B
  mockIC.run_test("call_greet self", call_greet, GREET_SELF, OUT_HELLO,
                  silent_on_trap, my_principal);
  mockIC.expect_call(CANISTER_B, "greet",
                     MockIC::MockResponse::reply(OUT_HI_B));
  mockIC.run_test("call_greet B", call_greet, GREET_B, OUT_HI_B, silent_on_trap,
                  my_principal);

  // Rejects: by the callee, by a trap in the callee, by the IC
  run_text_test(mockIC, "call_reject self", call_reject, P_SELF,
                "4:rejected on purpose", my_principal);
  run_text_test(mockIC, "call_reject B (no response programmed)", call_reject,
                P_B, "3:Mock IC: no response", my_principal);
  mockIC.expect_call(CANISTER_B, "reject_me",
                     MockIC::MockResponse::reject(2, "try again"));
  run_text_test(mockIC, "call_reject B (SYS_TRANSIENT)", call_reject, P_B,
                "2:try again", my_principal);
  run_text_test(mockIC, "call_trap self", call_trap, P_SELF, "5", my_principal);

  // A call that is not performed: the coroutine resumes with the error
  mockIC.set_next_call_perform_rc(2);
  run_text_test(mockIC, "call_reject, call_perform fails", call_reject, P_SELF,
                "0:", my_principal);
  __uint128_t balance = mockIC.get_canister_self_cycle_balance();
  mockIC.sub_cycles_balance(balance);
  run_text_test(mockIC, "call_reject, insufficient liquid balance", call_reject,
                P_SELF, "0:", my_principal);
  mockIC.add_cycles_balance(balance);

  // Cycles: half accepted, half refunded, the balance back where it was
  mockIC.run_test("call_with_cycles self", call_with_cycles, CYCLES_SELF,
                  OUT_CYCLES, silent_on_trap, my_principal);
  if (mockIC.get_canister_self_cycle_balance() != balance) {
    std::cout << "Test 'call_with_cycles self' Failed: balance changed\n";
    exit(1);
  }

  // A best-effort call that times out
  mockIC.expect_call(CANISTER_B, "slow", MockIC::MockResponse::timeout());
  run_text_test(mockIC, "call_bounded B (timeout)", call_bounded, BOUNDED_B,
                "6:", my_principal);

  // A best-effort call to the canister itself: slow() checks it sees a
  // deadline
  run_text_test(mockIC, "call_bounded self", call_bounded,
                "4449444c0003687979010a0000000000c0115a01013c00000003000000",
                "replied", my_principal);

  // Two calls, one after the other
  run_text_test(mockIC, "call_sequence self", call_sequence, GREET_SELF,
                "Hello, Hello, C++!!", my_principal);
  mockIC.expect_call(CANISTER_B, "greet",
                     MockIC::MockResponse::reply(OUT_HI_B));
  run_text_test(mockIC, "call_sequence B", call_sequence, GREET_B,
                "Hello, Hi from B!", my_principal);

  // A chain of calls: each callee calls the next before it replies
  const std::string self{MOCKIC_CANISTER_SELF};
  run_text_test(mockIC, "call_chain self > self > self", call_chain,
                chain_args({self, self}, "C++"),
                self + " > " + self + " > " + self + ": Hello, C++!",
                my_principal);
  run_text_test(mockIC, "call_chain end", call_chain, chain_args({}, "C++"),
                self + ": Hello, C++!", my_principal);

  // A long computation in chunks, with a yield after each
  {
    std::string out_hex;
    mockIC.run_test("compute alone", compute,
                    compute_args(9, 42, 200'000, true), "", silent_on_trap,
                    my_principal, &out_hex);
    uint64_t result{0}, call_instructions{0}, max_message{0};
    uint32_t chunks{0};
    CandidArgs out;
    out.append(CandidTypeNat64{&result});
    out.append(CandidTypeNat32{&chunks});
    out.append(CandidTypeNat64{&call_instructions});
    out.append(CandidTypeNat64{&max_message});
    decode(out_hex, out);
    check(result == lcg(42, 200'000) && chunks == 4,
          "compute alone: the right result, in 4 chunks");
  }
  // Jobs that run at the same time: right, because every chunk reloads its
  // job; wrong when the chunks skip the reload
  check(run_interleaved_jobs(mockIC, true, my_principal) == 3,
        "interleaved jobs, with reload: all 3 right");
  check(run_interleaved_jobs(mockIC, false, my_principal) < 3,
        "interleaved jobs, without reload: not all right");

  // A oneway call: performed, no response
  mockIC.run_test("call_oneway self", call_oneway, P_SELF, "4449444c00017e01",
                  silent_on_trap, my_principal);

  // Yields: the caller survives them, each resumes in a new message
  run_text_test(mockIC, "yield_loop 3", yield_loop, N3,
                my_principal + " after 3 yields", my_principal);
  mockIC.run_test("budget_probe 2", budget_probe, N2, "", silent_on_trap,
                  my_principal);

  // A test can feed its own instruction counter
  mockIC.set_performance_counter(
      [](uint32_t type) -> uint64_t { return 1000 + type; });
  {
    std::string out_hex;
    mockIC.run_test("budget_probe 2, own counter", budget_probe, N2, "",
                    silent_on_trap, my_principal, &out_hex);
    std::vector<uint64_t> message, call;
    CandidArgs out;
    out.append(CandidTypeVecNat64{&message});
    out.append(CandidTypeVecNat64{&call});
    decode(out_hex, out);
    check(message == std::vector<uint64_t>{1000, 1000} &&
              call == std::vector<uint64_t>{1001, 1001},
          "set_performance_counter: the counter the test fed");
  }
  mockIC.clear_performance_counter();

  // A stopping canister: yield_fresh_budget returns CanisterStopping, so
  // the loop replies with what it has.
  // - Stopping before the yield: no self-call is made.
  mockIC.set_canister_status(MOCKIC_CANISTER_STATUS_STOPPING);
  run_text_test(mockIC, "yield_until_stop, stopping before the yield",
                yield_until_stop, N3, "stopping after 0 yields", my_principal);
  check(mockIC.pending_calls() == 0, "stopping: no self-call queued");
  mockIC.set_canister_status(MOCKIC_CANISTER_STATUS_RUNNING);
  run_text_test(mockIC, "yield_until_stop, running", yield_until_stop, N3,
                "done after 3 yields", my_principal);
  // - Stopping while the self-call is queued: the IC rejects it.
  //   Each yield is 2 events: the self-call, then its response.
  mockIC.set_auto_drain(false);
  {
    MockIC::CallHandle h = mockIC.start_call(
        "yield_until_stop 100", yield_until_stop, N100, my_principal);
    for (int i = 0; i < 4; ++i)
      mockIC.drain_one();
    mockIC.set_canister_status(MOCKIC_CANISTER_STATUS_STOPPING);
    mockIC.drain();
    mockIC.set_canister_status(MOCKIC_CANISTER_STATUS_RUNNING);
    check(mockIC.call_done(h) && !mockIC.was_rejected(h) &&
              text_of(mockIC.reply_hex(h)) == "stopping after 2 yields",
          "yield_until_stop, stopping while the self-call is queued");
  }
  // - Stopping while the response is queued: it is still delivered, and the
  //   next yield sees the stop.
  {
    MockIC::CallHandle h = mockIC.start_call(
        "yield_until_stop 100", yield_until_stop, N100, my_principal);
    for (int i = 0; i < 5; ++i)
      mockIC.drain_one();
    mockIC.set_canister_status(MOCKIC_CANISTER_STATUS_STOPPING);
    mockIC.drain();
    mockIC.set_canister_status(MOCKIC_CANISTER_STATUS_RUNNING);
    check(text_of(mockIC.reply_hex(h)) == "stopping after 3 yields",
          "yield_until_stop, stopping while the response is queued");
  }
  // The reply of each interleaved call, and a call that traps
  {
    MockIC::CallHandle a =
        mockIC.start_call("yield_loop 2", yield_loop, N2, my_principal);
    MockIC::CallHandle b =
        mockIC.start_call("yield_loop 3", yield_loop, N3, my_principal);
    MockIC::CallHandle c =
        mockIC.start_call("trap_me", trap_me, EMPTY, my_principal);
    check(!mockIC.call_done(a) && !mockIC.call_done(b),
          "start_call: not done before drain");
    mockIC.drain();
    check(text_of(mockIC.reply_hex(a)) == my_principal + " after 2 yields" &&
              text_of(mockIC.reply_hex(b)) == my_principal + " after 3 yields",
          "start_call: the reply of each interleaved call");
    check(mockIC.call_done(c) && mockIC.was_rejected(c) &&
              mockIC.reject_message(c).rfind("trapped: ", 0) == 0,
          "start_call: a trap is a reject");
  }
  mockIC.set_auto_drain(true);

  // The code after a co_await traps: the cleanup frees the frame, and the
  // call is no longer in flight. (The Mock IC does not roll back g_after.)
  mockIC.run_trap_test("call_then_trap self", call_then_trap, P_SELF,
                       silent_on_trap, my_principal);
  // (before, after, destroyed, calls_in_flight, held_done)
  mockIC.run_test("get_state after call_then_trap", get_state, EMPTY,
                  "4449444c0005787878797e01000000000000000100000000000000"
                  "01000000000000000000000000",
                  silent_on_trap, my_principal);

  // A coroutine that awaits another coroutine: IC_Task
  run_text_test(mockIC, "task_greet self", task_greet, GREET_SELF,
                "Hello, C++!", my_principal);
  mockIC.expect_call(CANISTER_B, "greet",
                     MockIC::MockResponse::reply(OUT_HI_B));
  run_text_test(mockIC, "task_greet B", task_greet, GREET_B, "Hi from B",
                my_principal);
  run_text_test(mockIC, "task_nested self", task_nested, GREET_SELF,
                "[Hello, Hello, C++!!]", my_principal);
  mockIC.run_test("task_not_awaited: the task never ran", task_not_awaited,
                  EMPTY, "4449444c0001780000000000000000", silent_on_trap,
                  my_principal);
  {
    // A trap in a task, after its co_await: the cleanup frees the task's
    // frame and the root's. (The Mock IC does not roll back g_after.)
    State before = read_state(mockIC, my_principal);
    mockIC.run_trap_test("task_then_trap self", task_then_trap, P_SELF,
                         silent_on_trap, my_principal);
    State after = read_state(mockIC, my_principal);
    check(after.destroyed == before.destroyed + 2 && after.in_flight == 0,
          "task_then_trap: both frames freed, no call in flight");
  }

  // Several calls at once: IC_CallAll
  check_call_all(
      mockIC, "call_all self",
      calls_args({self, self, self}, {"greet", "reject_me", "greet"}),
      {"ok:Hello, C++!", "rej:4", "ok:Hello, C++!"}, my_principal);
  mockIC.expect_call(CANISTER_B, "greet",
                     MockIC::MockResponse::reply(OUT_HI_B));
  check_call_all(mockIC, "call_all self and B",
                 calls_args({CANISTER_B, self}, {"greet", "greet"}),
                 {"ok:Hi from B", "ok:Hello, C++!"}, my_principal);
  // A call that is not performed gets its error; the others go ahead
  mockIC.set_next_call_perform_rc(1);
  check_call_all(mockIC, "call_all, the first call not performed",
                 calls_args({self, self}, {"greet", "greet"}),
                 {"err:ic0.call_perform failed with code 1", "ok:Hello, C++!"},
                 my_principal);
  // No call performed: the coroutine continues without suspending
  mockIC.set_next_call_perform_rc(1);
  check_call_all(mockIC, "call_all, no call performed",
                 calls_args({self}, {"greet"}),
                 {"err:ic0.call_perform failed with code 1"}, my_principal);
  check_call_all(mockIC, "call_all, no calls", calls_args({}, {}), {},
                 my_principal);
  {
    State before = read_state(mockIC, my_principal);
    mockIC.run_trap_test("call_all_then_trap self", call_all_then_trap,
                         calls_args({self, self}, {"greet", "reject_me"}),
                         silent_on_trap, my_principal);
    State after = read_state(mockIC, my_principal);
    check(after.destroyed == before.destroyed + 1 && after.in_flight == 0,
          "call_all_then_trap: the frame freed, no call in flight");
  }

  // Composite queries: queries that call queries
  run_text_test(mockIC, "cq_greet self", cq_greet, GREET_SELF, "Hello, C++!",
                my_principal);
  mockIC.expect_call(CANISTER_B, "greet_q",
                     MockIC::MockResponse::reply(OUT_HI_B));
  run_text_test(mockIC, "cq_greet B", cq_greet, GREET_B, "Hi from B",
                my_principal);
  {
    CandidArgs in;
    in.append(CandidTypeVecPrincipal{std::vector<std::string>{self, self}});
    VecBytes in_bytes = CandidSerialize(in).get_B();
    std::string out_hex;
    mockIC.run_test("cq_all self", cq_all, in_bytes.as_hex_string(), "",
                    silent_on_trap, my_principal, &out_hex);
    std::vector<std::string> out;
    CandidArgs args;
    args.append(CandidTypeVecText{&out});
    decode(out_hex, args);
    check(out == std::vector<std::string>{"ok:Hello, C++!", "ok:Hello, C++!"},
          "cq_all self: both calls replied");
  }
  // A composite query can't call an update method, and an update can't call
  // a composite query
  run_text_test(mockIC, "cq_call_update self", cq_call_update, P_SELF, "rej:5",
                my_principal);
  run_text_test(mockIC, "call_cq self", call_cq, P_SELF, "rej:5", my_principal);
  {
    // In a composite query's callback: no certificate, not replicated
    CandidArgs in;
    in.append(CandidTypePrincipal{self});
    in.append(CandidTypeText{"probe"});
    VecBytes in_bytes = CandidSerialize(in).get_B();
    run_text_test(mockIC, "cq_in_callback probe", cq_in_callback,
                  in_bytes.as_hex_string(),
                  "certificate: 0 then 0; replicated: 0 then 0", my_principal);
  }
  for (const std::string what : {"timer", "certify"}) {
    // Refused in a composite query's callback: the callback traps, and the
    // query does not reply
    CandidArgs in;
    in.append(CandidTypePrincipal{self});
    in.append(CandidTypeText{what});
    VecBytes in_bytes = CandidSerialize(in).get_B();
    mockIC.run_trap_test("cq_in_callback " + what + ": refused", cq_in_callback,
                         in_bytes.as_hex_string(), silent_on_trap,
                         my_principal);
  }
  {
    // A composite query can't send cycles: it traps before the call
    CandidArgs in;
    in.append(CandidTypePrincipal{self});
    in.append(CandidTypeText{"cycles"});
    VecBytes in_bytes = CandidSerialize(in).get_B();
    mockIC.run_trap_test("cq_in_callback cycles", cq_in_callback,
                         in_bytes.as_hex_string(), silent_on_trap,
                         my_principal);
  }
  {
    State before = read_state(mockIC, my_principal);
    mockIC.run_trap_test("cq_then_trap self", cq_then_trap, P_SELF,
                         silent_on_trap, my_principal);
    State after = read_state(mockIC, my_principal);
    check(after.destroyed == before.destroyed + 1 && after.in_flight == 0,
          "cq_then_trap: the frame freed, never counted in flight");
  }

  // The same logic as an update and as a composite query; a composite query
  // calling a composite query
  run_text_test(mockIC, "info self", info, P_SELF, "ok:Hello, info!",
                my_principal);
  run_text_test(mockIC, "info_cq self", info_cq, P_SELF, "ok:Hello, info!",
                my_principal);
  run_text_test(mockIC, "cq_via self self", cq_via, P_SELF_SELF,
                "ok:Hello, info!", my_principal);

  // A timer can't call a composite query, but can call its update variant.
  // The Mock IC does not fire timers: the test calls canister_global_timer.
  mockIC.run_test("timer_call info_cq", timer_call, TIMER_INFO_CQ, EMPTY,
                  silent_on_trap, my_principal);
  mockIC.run_test("canister_global_timer (info_cq)", canister_global_timer,
                  EMPTY, "", silent_on_trap, my_principal);
  run_text_test(mockIC, "timer_result info_cq", timer_result, EMPTY, "rej:5",
                my_principal);
  mockIC.run_test("timer_call info", timer_call, TIMER_INFO, EMPTY,
                  silent_on_trap, my_principal);
  mockIC.run_test("canister_global_timer (info)", canister_global_timer, EMPTY,
                  "", silent_on_trap, my_principal);
  run_text_test(mockIC, "timer_result info", timer_result, EMPTY,
                "ok:Hello, info!", my_principal);

  // An upgrade while a call is in flight is refused
  mockIC.set_auto_drain(false);
  mockIC.run_test("start_held_call self", start_held_call, P_SELF,
                  "4449444c0000", silent_on_trap, my_principal);
  mockIC.run_trap_test("canister_pre_upgrade (default), call in flight",
                       icpp_default_pre_upgrade, EMPTY, silent_on_trap,
                       my_principal);
  mockIC.run_trap_test("canister_pre_upgrade (IC_API), call in flight",
                       native_pre_upgrade_with_ic_api, EMPTY, silent_on_trap,
                       my_principal);
  mockIC.run_test("release", release, EMPTY, "4449444c0000", silent_on_trap,
                  my_principal);
  mockIC.drain(); // hold() sees the release and returns
  mockIC.set_auto_drain(true);
  mockIC.run_test("canister_pre_upgrade (default), no call in flight",
                  icpp_default_pre_upgrade, EMPTY, "", silent_on_trap,
                  my_principal);
  {
    State state = read_state(mockIC, my_principal);
    check(state.in_flight == 0 && state.held_done,
          "get_state after the held call: done, no call in flight");
  }

  // Two coroutines in one call: a trap in one fails the call, also when the
  // other replies, or ends without a reply, later
  for (bool reply : {false, true}) {
    CandidArgs in;
    in.append(CandidTypePrincipal{self});
    in.append(CandidTypeBool{reply});
    VecBytes in_bytes = CandidSerialize(in).get_B();
    mockIC.run_trap_test(std::string("two_coroutines, the other ") +
                             (reply ? "replies" : "does not reply"),
                         two_coroutines, in_bytes.as_hex_string(),
                         silent_on_trap, my_principal);
  }

  // start_call: the reply of each call, and a stale handle
  mockIC.set_auto_drain(false);
  {
    // A call that traps after a co_await: the caller gets a reject
    MockIC::CallHandle h = mockIC.start_call(
        "call_then_trap self", call_then_trap, P_SELF, my_principal);
    mockIC.drain();
    check(mockIC.call_done(h) && mockIC.was_rejected(h),
          "start_call: a trap after a co_await is a reject");

    // Two calls on a stopping canister: neither queues a call, and each
    // handle still reads its own reply
    mockIC.set_canister_status(MOCKIC_CANISTER_STATUS_STOPPING);
    MockIC::CallHandle a = mockIC.start_call(
        "yield_until_stop 3", yield_until_stop, N3, my_principal);
    MockIC::CallHandle b =
        mockIC.start_call("yield_loop 2", yield_loop, N2, my_principal);
    mockIC.set_canister_status(MOCKIC_CANISTER_STATUS_RUNNING);
    check(text_of(mockIC.reply_hex(a)) == "stopping after 0 yields" &&
              mockIC.was_rejected(b) && mockIC.call_done(h),
          "start_call: handles stay valid when nothing is queued");

    // A run_test with nothing queued forgets the calls: an old handle traps
    mockIC.run_test("get_state", get_state, EMPTY, "", silent_on_trap,
                    my_principal);
    bool trapped{false};
    try {
      mockIC.reply_hex(a);
    } catch (const std::exception &) {
      trapped = true;
    }
    check(trapped, "start_call: a stale handle traps");
  }
  mockIC.set_auto_drain(true);

  // returns 1 if any tests failed
  return mockIC.test_summary();
}
