// Inter-canister calls (ic_async.h): the callee methods, and the caller
// methods that call them - on another canister, or on the canister itself.

#include "my_canister.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

#include "ic0.h"
#include "ic_api.h"

namespace {
// Committed state, read by get_state
uint64_t g_before{0};    // call_then_trap, before its co_await
uint64_t g_after{0};     // call_then_trap, after its co_await (rolled back)
uint64_t g_destroyed{0}; // call_then_trap frames freed (by the cleanup)
bool g_released{false};  // hold() returns once release() was called
bool g_held_done{false}; // the held call of start_held_call returned
uint64_t g_task_ran{0};  // bump_task ran

// Counts the frames of call_then_trap that are freed.
struct FrameGuard {
  ~FrameGuard() { ++g_destroyed; }
};

CandidTypePrincipal principal_arg(IC_API &ic_api, CandidArgs &args,
                                  std::string &text) {
  ic_api.from_wire(args);
  return CandidTypePrincipal(text);
}

// "<code>:<message>" of a failed call
std::string error_text(const IC_CallError &error) {
  return std::to_string(static_cast<uint32_t>(error.reject_code)) + ":" +
         error.reject_message;
}
// A long computation: an LCG iterated n times from a seed. The state of each
// job is saved between its chunks, like llama's prompt cache per caller.
struct LcgState {
  uint64_t x{0}; // the LCG value
  uint64_t i{0}; // the steps done
};
std::unordered_map<uint32_t, LcgState> g_jobs;
// The shared work area every chunk computes in, like llama's model context:
// another job's chunk can run in between, so a chunk must reload its job.
LcgState g_work;
std::vector<uint32_t> g_run_log; // the job of every chunk, in order of running

// x = a * x + c (mod 2^64), with Knuth's MMIX constants
constexpr uint64_t LCG_A = 6364136223846793005ULL;
constexpr uint64_t LCG_C = 1442695040888963407ULL;
// Steps between two reads of the instruction counter
constexpr uint64_t LCG_BLOCK = 65536;

// Runs blocks of steps on g_work until n steps are done, or the message
// executed `chunk_instructions` instructions.
void lcg_chunk(uint64_t n, uint64_t chunk_instructions) {
  do {
    uint64_t end = std::min(n, g_work.i + LCG_BLOCK);
    uint64_t x = g_work.x;
    for (uint64_t i = g_work.i; i < end; ++i)
      x = LCG_A * x + LCG_C;
    g_work = {x, end};
  } while (g_work.i < n && IC_API::performance_counter(
                               IC_API::PerformanceCounterType::Instructions) <
                               chunk_instructions);
}
} // namespace

// ---------------------------------------------------------------------------
// Callee methods

// '("C++")' -> '("Hello, C++!")'
void greet() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  std::string name;
  ic_api.from_wire(CandidTypeText{&name});
  ic_api.to_wire(CandidTypeText{"Hello, " + name + "!"});
}

void reject_me() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  ic_api.from_wire();
  std::string message = "rejected on purpose";
  ic0_msg_reject(reinterpret_cast<uintptr_t>(message.data()),
                 (uint32_t)message.size());
}

void trap_me() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  IC_API::trap("trapped on purpose");
}

// Accepts half of the cycles sent along: '()' -> '(accepted : nat64)'
void accept_half() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  ic_api.from_wire();
  __uint128_t accepted =
      ic_api.accept_msg_cycles(ic_api.get_msg_cycles_available() / 2);
  ic_api.to_wire(CandidTypeNat64{(uint64_t)accepted});
}

// Replies after n self-yields: '(n : nat32)' -> '()'. Called by a
// bounded-wait call, so it - and its callbacks - see a deadline.
IC_Async slow_async() {
  IC_API ic_api(CanisterUpdate{"slow"}, false);
  uint32_t n{0};
  ic_api.from_wire(CandidTypeNat32{&n});
  for (uint32_t i = 0; i < n; ++i) {
    IC_CallResult<void> r = co_await ic_api.yield_fresh_budget();
    r.value();
    if (ic_api.get_msg_deadline() == 0) {
      IC_API::trap("slow: no deadline, but called by a bounded-wait call");
    }
  }
  ic_api.to_wire();
}
void slow() { slow_async(); }

// Replies once release() was called, or after 10 minutes - a safety cap,
// long enough that a slow CI runner never reaches it during the upgrade test:
// '()' -> '()'
IC_Async hold_async() {
  IC_API ic_api(CanisterUpdate{"hold"}, false);
  ic_api.from_wire();
  g_released = false;
  uint64_t until = IC_API::time() + 600'000'000'000;
  while (!g_released && IC_API::time() < until) {
    IC_CallResult<void> r = co_await ic_api.yield_fresh_budget();
    r.value();
  }
  ic_api.to_wire();
}
void hold() { hold_async(); }

void release() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  ic_api.from_wire();
  g_released = true;
  ic_api.to_wire();
}

// ---------------------------------------------------------------------------
// Caller methods

// '(callee, "C++")' -> '("Hello, C++!")'
IC_Async call_greet_async() {
  IC_API ic_api(CanisterUpdate{"call_greet"}, false);
  std::string callee_text, name;
  CandidArgs args;
  args.append(CandidTypePrincipal{&callee_text});
  args.append(CandidTypeText{&name});
  CandidTypePrincipal callee = principal_arg(ic_api, args, callee_text);

  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(callee, "greet")
          .with_arg(CandidTypeText{name});
  std::string greeting;
  r->decode(CandidTypeText{&greeting});
  ic_api.to_wire(CandidTypeText{greeting});
}
void call_greet() { call_greet_async(); }

// '(callee)' -> '("4:rejected on purpose")'
IC_Async call_reject_async() {
  IC_API ic_api(CanisterUpdate{"call_reject"}, false);
  std::string callee_text;
  CandidArgs args;
  args.append(CandidTypePrincipal{&callee_text});
  CandidTypePrincipal callee = principal_arg(ic_api, args, callee_text);

  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::unbounded_wait(callee, "reject_me");
  ic_api.to_wire(CandidTypeText{r ? "replied" : error_text(r.error())});
}
void call_reject() { call_reject_async(); }

// The callee traps: '(callee)' -> '("5:...")'
IC_Async call_trap_async() {
  IC_API ic_api(CanisterUpdate{"call_trap"}, false);
  std::string callee_text;
  CandidArgs args;
  args.append(CandidTypePrincipal{&callee_text});
  CandidTypePrincipal callee = principal_arg(ic_api, args, callee_text);

  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::unbounded_wait(callee, "trap_me");
  ic_api.to_wire(CandidTypeText{
      r ? "replied"
        : std::to_string(static_cast<uint32_t>(r.error().reject_code))});
}
void call_trap() { call_trap_async(); }

// The code after the co_await traps: what it did is rolled back, and the
// cleanup frees the frame. '(callee)' -> trap
IC_Async call_then_trap_async() {
  IC_API ic_api(CanisterUpdate{"call_then_trap"}, false);
  std::string callee_text;
  CandidArgs args;
  args.append(CandidTypePrincipal{&callee_text});
  CandidTypePrincipal callee = principal_arg(ic_api, args, callee_text);
  FrameGuard guard;

  ++g_before; // committed by the co_await
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::unbounded_wait(callee, "greet")
          .with_arg(CandidTypeText{"trap"});
  ++g_after; // rolled back by the trap
  IC_API::trap("call_then_trap: trapped on purpose after the co_await");
}
void call_then_trap() { call_then_trap_async(); }

// '(callee, cycles)' -> '(accepted, refunded)'
IC_Async call_with_cycles_async() {
  IC_API ic_api(CanisterUpdate{"call_with_cycles"}, false);
  std::string callee_text;
  uint64_t cycles{0};
  CandidArgs args;
  args.append(CandidTypePrincipal{&callee_text});
  args.append(CandidTypeNat64{&cycles});
  CandidTypePrincipal callee = principal_arg(ic_api, args, callee_text);

  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::unbounded_wait(callee, "accept_half")
          .with_cycles(cycles);
  uint64_t accepted{0};
  r->decode(CandidTypeNat64{&accepted});
  CandidArgs out;
  out.append(CandidTypeNat64{accepted});
  out.append(CandidTypeNat64{(uint64_t)r->cycles_refunded});
  ic_api.to_wire(out);
}
void call_with_cycles() { call_with_cycles_async(); }

// A best-effort call to slow(iterations) with a timeout:
// '(callee, timeout_seconds, iterations)' -> '("replied")' or '("6:...")'
IC_Async call_bounded_async() {
  IC_API ic_api(CanisterUpdate{"call_bounded"}, false);
  std::string callee_text;
  uint32_t timeout{0}, iterations{0};
  CandidArgs args;
  args.append(CandidTypePrincipal{&callee_text});
  args.append(CandidTypeNat32{&timeout});
  args.append(CandidTypeNat32{&iterations});
  CandidTypePrincipal callee = principal_arg(ic_api, args, callee_text);

  IC_CallResult<IC_CallReply> r = co_await IC_Call::bounded_wait(callee, "slow")
                                      .with_arg(CandidTypeNat32{iterations})
                                      .change_timeout(timeout);
  ic_api.to_wire(CandidTypeText{r ? "replied" : error_text(r.error())});
}
void call_bounded() { call_bounded_async(); }

// Two calls, one after the other: callee.greet(name), then a call to the
// canister itself with that reply: '(callee, "C++")' -> '("Hello, Hello, C++!!")'
IC_Async call_sequence_async() {
  IC_API ic_api(CanisterUpdate{"call_sequence"}, false);
  std::string callee_text, name;
  CandidArgs args;
  args.append(CandidTypePrincipal{&callee_text});
  args.append(CandidTypeText{&name});
  CandidTypePrincipal callee = principal_arg(ic_api, args, callee_text);

  std::string first, second;
  IC_CallResult<IC_CallReply> r1 =
      co_await IC_Call::bounded_wait(callee, "greet")
          .with_arg(CandidTypeText{name});
  r1->decode(CandidTypeText{&first});
  IC_CallResult<IC_CallReply> r2 =
      co_await IC_Call::bounded_wait(ic_api.get_canister_self(), "greet")
          .with_arg(CandidTypeText{first});
  r2->decode(CandidTypeText{&second});
  ic_api.to_wire(CandidTypeText{second});
}
void call_sequence() { call_sequence_async(); }

// A chain of calls: calls hops[0].call_chain(hops[1..], name), and each
// callee calls the next before it replies. A hop may be any canister,
// including one earlier in the chain:
// '(vec {b; c}, "C++")' on a -> '("a > b > c: Hello, C++!")'
IC_Async call_chain_async() {
  IC_API ic_api(CanisterUpdate{"call_chain"}, false);
  std::vector<std::string> hops;
  std::string name;
  CandidArgs args;
  args.append(CandidTypeVecPrincipal{&hops});
  args.append(CandidTypeText{&name});
  ic_api.from_wire(args);
  std::string me = ic_api.get_canister_self().get_text();
  if (hops.empty()) {
    ic_api.to_wire(CandidTypeText{me + ": Hello, " + name + "!"});
    co_return;
  }

  CandidArgs next;
  next.append(CandidTypeVecPrincipal{
      std::vector<std::string>(hops.begin() + 1, hops.end())});
  next.append(CandidTypeText{name});
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(CandidTypePrincipal{hops[0]}, "call_chain")
          .with_args(next);
  std::string rest;
  r->decode(CandidTypeText{&rest});
  ic_api.to_wire(CandidTypeText{me + " > " + rest});
}
void call_chain() { call_chain_async(); }

// ---------------------------------------------------------------------------
// Composition: a coroutine that awaits another coroutine (IC_Task), and
// several calls at once (IC_CallAll)

namespace {
// callee.greet(name), as a task
IC_Task<IC_CallResult<std::string>> greet_task(CandidTypePrincipal callee,
                                               std::string name) {
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(callee, "greet")
          .with_arg(CandidTypeText{name});
  if (!r) co_return r.error();
  std::string greeting;
  r->decode(CandidTypeText{&greeting});
  co_return greeting;
}

// Two levels: a task awaiting tasks
IC_Task<std::string> twice_task(CandidTypePrincipal callee, std::string name) {
  IC_CallResult<std::string> first = co_await greet_task(callee, name);
  IC_CallResult<std::string> second = co_await greet_task(callee, *first);
  co_return *second;
}

// A task that finishes without making a call
IC_Task<std::string> bracket_task(std::string s) { co_return "[" + s + "]"; }

IC_Task<> bump_task() {
  ++g_task_ran;
  co_return;
}

// Traps after its co_await: the cleanup destroys the whole chain
IC_Task<> trap_task(CandidTypePrincipal callee) {
  FrameGuard guard;
  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(callee, "greet")
          .with_arg(CandidTypeText{"trap"});
  ++g_after; // rolled back by the trap
  IC_API::trap("task_then_trap: trapped on purpose after the co_await");
}

// "ok:<greeting>", "rej:<code>" or "err:<message>" of one call
std::string outcome(IC_CallResult<IC_CallReply> &r) {
  if (!r) {
    if (r.error().kind == IC_CallError::Kind::CallRejected) {
      return "rej:" +
             std::to_string(static_cast<uint32_t>(r.error().reject_code));
    }
    return "err:" + r.error().to_string();
  }
  std::string greeting;
  r->decode(CandidTypeText{&greeting});
  return "ok:" + greeting;
}

// The callees of call_all and call_all_then_trap: '(vec principal, vec text)'
std::vector<IC_Call> calls_arg(IC_API &ic_api) {
  std::vector<std::string> callees, methods;
  CandidArgs args;
  args.append(CandidTypeVecPrincipal{&callees});
  args.append(CandidTypeVecText{&methods});
  ic_api.from_wire(args);
  if (callees.size() != methods.size()) {
    IC_API::trap("calls_arg: callees and methods must have the same length");
  }
  std::vector<IC_Call> calls;
  for (size_t i = 0; i < callees.size(); ++i) {
    IC_Call call =
        IC_Call::bounded_wait(CandidTypePrincipal{callees[i]}, methods[i]);
    if (methods[i] == "greet") call.with_arg(CandidTypeText{"C++"});
    calls.push_back(call);
  }
  return calls;
}
} // namespace

// '(callee, "C++")' -> '("Hello, C++!")'
IC_Async task_greet_async() {
  IC_API ic_api(CanisterUpdate{"task_greet"}, false);
  std::string callee_text, name;
  CandidArgs args;
  args.append(CandidTypePrincipal{&callee_text});
  args.append(CandidTypeText{&name});
  CandidTypePrincipal callee = principal_arg(ic_api, args, callee_text);

  IC_CallResult<std::string> greeting = co_await greet_task(callee, name);
  ic_api.to_wire(
      CandidTypeText{greeting ? *greeting : error_text(greeting.error())});
}
void task_greet() { task_greet_async(); }

// '(callee, "C++")' -> '("[Hello, Hello, C++!!]")'
IC_Async task_nested_async() {
  IC_API ic_api(CanisterUpdate{"task_nested"}, false);
  std::string callee_text, name;
  CandidArgs args;
  args.append(CandidTypePrincipal{&callee_text});
  args.append(CandidTypeText{&name});
  CandidTypePrincipal callee = principal_arg(ic_api, args, callee_text);

  std::string twice = co_await twice_task(callee, name);
  std::string bracketed = co_await bracket_task(twice);
  ic_api.to_wire(CandidTypeText{bracketed});
}
void task_nested() { task_nested_async(); }

// A task that traps after its co_await: what it and the root did after the
// co_await is rolled back, and the cleanup frees both frames.
// '(callee)' -> trap
IC_Async task_then_trap_async() {
  IC_API ic_api(CanisterUpdate{"task_then_trap"}, false);
  std::string callee_text;
  CandidArgs args;
  args.append(CandidTypePrincipal{&callee_text});
  CandidTypePrincipal callee = principal_arg(ic_api, args, callee_text);
  FrameGuard guard;

  ++g_before; // committed by the co_await
  co_await trap_task(callee);
}
void task_then_trap() { task_then_trap_async(); }

// A task that is never awaited never runs: '()' -> '(times bump_task ran)'
IC_Async task_not_awaited_async() {
  IC_API ic_api(CanisterUpdate{"task_not_awaited"}, false);
  ic_api.from_wire();
  {
    IC_Task<> unused = bump_task();
  }
  ic_api.to_wire(CandidTypeNat64{g_task_ran});
  co_return;
}
void task_not_awaited() { task_not_awaited_async(); }

// Several calls at once, results in the order of the calls:
// '(vec {a; b}, vec {"greet"; "reject_me"})' ->
// '(vec {"ok:Hello, C++!"; "rej:4"})'
IC_Async call_all_async() {
  IC_API ic_api(CanisterUpdate{"call_all"}, false);
  std::vector<IC_Call> calls = calls_arg(ic_api);

  std::vector<IC_CallResult<IC_CallReply>> results = co_await IC_CallAll(calls);
  std::vector<std::string> out;
  for (IC_CallResult<IC_CallReply> &r : results)
    out.push_back(outcome(r));
  ic_api.to_wire(CandidTypeVecText{out});
}
void call_all() { call_all_async(); }

// The code after a co_await IC_CallAll traps: the cleanup frees the frame.
// '(vec principal, vec text)' -> trap
IC_Async call_all_then_trap_async() {
  IC_API ic_api(CanisterUpdate{"call_all_then_trap"}, false);
  std::vector<IC_Call> calls = calls_arg(ic_api);
  FrameGuard guard;

  ++g_before; // committed by the co_await
  std::vector<IC_CallResult<IC_CallReply>> results = co_await IC_CallAll(calls);
  ++g_after; // rolled back by the trap
  IC_API::trap("call_all_then_trap: trapped on purpose after the co_await");
}
void call_all_then_trap() { call_all_then_trap_async(); }

// ---------------------------------------------------------------------------
// Composite queries: queries that call queries

// A query, for the composite queries to call: '("C++")' -> '("Hello, C++!")'
void greet_q() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::string name;
  ic_api.from_wire(CandidTypeText{&name});
  ic_api.to_wire(CandidTypeText{"Hello, " + name + "!"});
}

// A composite query, with no arguments: '()' -> '("composite")'
void cq_name() {
  IC_API ic_api(CanisterCompositeQuery{std::string(__func__)}, false);
  ic_api.from_wire();
  ic_api.to_wire(CandidTypeText{"composite"});
}

// Calls callee.greet_q: '(callee, "C++")' -> '("Hello, C++!")'
IC_Async cq_greet_async() {
  IC_API ic_api(CanisterCompositeQuery{"cq_greet"}, false);
  std::string callee_text, name;
  CandidArgs args;
  args.append(CandidTypePrincipal{&callee_text});
  args.append(CandidTypeText{&name});
  CandidTypePrincipal callee = principal_arg(ic_api, args, callee_text);

  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(callee, "greet_q")
          .with_arg(CandidTypeText{name});
  std::string greeting;
  if (r) r->decode(CandidTypeText{&greeting});
  ic_api.to_wire(CandidTypeText{r ? greeting : error_text(r.error())});
}
void cq_greet() { cq_greet_async(); }

// greet_q on several canisters at once:
// '(vec {a; b})' -> '(vec {"ok:Hello, C++!"; "ok:Hello, C++!"})'
IC_Async cq_all_async() {
  IC_API ic_api(CanisterCompositeQuery{"cq_all"}, false);
  std::vector<std::string> callees;
  ic_api.from_wire(CandidTypeVecPrincipal{&callees});
  std::vector<IC_Call> calls;
  for (const std::string &callee : callees) {
    calls.push_back(
        IC_Call::bounded_wait(CandidTypePrincipal{callee}, "greet_q")
            .with_arg(CandidTypeText{"C++"}));
  }
  std::vector<IC_CallResult<IC_CallReply>> results = co_await IC_CallAll(calls);
  std::vector<std::string> out;
  for (IC_CallResult<IC_CallReply> &r : results)
    out.push_back(outcome(r));
  ic_api.to_wire(CandidTypeVecText{out});
}
void cq_all() { cq_all_async(); }

// A composite query calling an update method is rejected:
// '(callee)' -> '("rej:<code>")'
IC_Async cq_call_update_async() {
  IC_API ic_api(CanisterCompositeQuery{"cq_call_update"}, false);
  std::string callee_text;
  CandidArgs args;
  args.append(CandidTypePrincipal{&callee_text});
  CandidTypePrincipal callee = principal_arg(ic_api, args, callee_text);

  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(callee, "greet")
          .with_arg(CandidTypeText{"C++"});
  ic_api.to_wire(CandidTypeText{outcome(r)});
}
void cq_call_update() { cq_call_update_async(); }

// An update calling a composite query is rejected: '(callee)' -> '("rej:5")'
IC_Async call_cq_async() {
  IC_API ic_api(CanisterUpdate{"call_cq"}, false);
  std::string callee_text;
  CandidArgs args;
  args.append(CandidTypePrincipal{&callee_text});
  CandidTypePrincipal callee = principal_arg(ic_api, args, callee_text);

  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(callee, "cq_name");
  ic_api.to_wire(CandidTypeText{r ? "replied" : outcome(r)});
}
void call_cq() { call_cq_async(); }

// What a composite query and the callback of its call may do:
// '(callee, what)' -> trap, for what = "timer", "certify" or "cycles"
// '(callee, "probe")' ->
//   '("certificate: 1 then 0; replicated: 0 then 0")'
IC_Async cq_in_callback_async() {
  IC_API ic_api(CanisterCompositeQuery{"cq_in_callback"}, false);
  std::string callee_text, what;
  CandidArgs args;
  args.append(CandidTypePrincipal{&callee_text});
  args.append(CandidTypeText{&what});
  CandidTypePrincipal callee = principal_arg(ic_api, args, callee_text);

  bool certificate_before = ic_api.data_certificate_present();
  bool replicated_before = IC_API::in_replicated_execution();
  IC_Call call =
      IC_Call::bounded_wait(callee, "greet_q").with_arg(CandidTypeText{"C++"});
  if (what == "cycles") call.with_cycles(1'000);
  IC_CallResult<IC_CallReply> r = co_await call;
  if (what == "timer") IC_API::set_timer(1'000'000'000, [] {});
  if (what == "certify") ic_api.set_certified_data({1, 2, 3});
  ic_api.to_wire(CandidTypeText{
      "certificate: " + std::to_string(certificate_before) + " then " +
      std::to_string(ic_api.data_certificate_present()) +
      "; replicated: " + std::to_string(replicated_before) + " then " +
      std::to_string(IC_API::in_replicated_execution())});
}
void cq_in_callback() { cq_in_callback_async(); }

// The code after the co_await traps: the cleanup frees the frame.
// '(callee)' -> trap
IC_Async cq_then_trap_async() {
  IC_API ic_api(CanisterCompositeQuery{"cq_then_trap"}, false);
  std::string callee_text;
  CandidArgs args;
  args.append(CandidTypePrincipal{&callee_text});
  CandidTypePrincipal callee = principal_arg(ic_api, args, callee_text);
  FrameGuard guard;

  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::bounded_wait(callee, "greet_q")
          .with_arg(CandidTypeText{"C++"});
  IC_API::trap("cq_then_trap: trapped on purpose after the co_await");
}
void cq_then_trap() { cq_then_trap_async(); }

// A call without a response: '(callee)' -> '(true)'
void call_oneway() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  std::string callee_text;
  CandidArgs args;
  args.append(CandidTypePrincipal{&callee_text});
  CandidTypePrincipal callee = principal_arg(ic_api, args, callee_text);
  IC_CallResult<void> r = IC_Call::unbounded_wait(callee, "greet")
                              .with_arg(CandidTypeText{"x"})
                              .oneway();
  ic_api.to_wire(CandidTypeBool{r.has_value()});
}

// n self-yields; the caller and the arguments survive them:
// '(n)' -> '("<caller> after <n> yields")'
IC_Async yield_loop_async() {
  IC_API ic_api(CanisterUpdate{"yield_loop"}, false);
  uint32_t n{0};
  ic_api.from_wire(CandidTypeNat32{&n});
  for (uint32_t i = 0; i < n; ++i) {
    IC_CallResult<void> r = co_await ic_api.yield_fresh_budget();
    r.value();
  }
  ic_api.to_wire(CandidTypeText{ic_api.get_caller().get_text() + " after " +
                                std::to_string(n) + " yields"});
}
void yield_loop() { yield_loop_async(); }

// The instructions of each message, and of the whole call, after each yield:
// '(n)' -> '(vec message_instructions, vec call_instructions)'
IC_Async budget_probe_async() {
  IC_API ic_api(CanisterUpdate{"budget_probe"}, false);
  uint32_t n{0};
  ic_api.from_wire(CandidTypeNat32{&n});
  std::vector<uint64_t> message, call;
  for (uint32_t i = 0; i < n; ++i) {
    // Some work, so every message executes a measurable number of
    // instructions
    volatile uint64_t sum = 0;
    for (uint64_t k = 0; k < 1'000'000; ++k)
      sum += k;
    message.push_back(IC_API::performance_counter(
        IC_API::PerformanceCounterType::Instructions));
    call.push_back(IC_API::performance_counter(
        IC_API::PerformanceCounterType::CallContextInstructions));
    IC_CallResult<void> r = co_await ic_api.yield_fresh_budget();
    r.value();
  }
  CandidArgs out;
  out.append(CandidTypeVecNat64{message});
  out.append(CandidTypeVecNat64{call});
  ic_api.to_wire(out);
}
void budget_probe() { budget_probe_async(); }

// Replies right away, then holds a call to callee.hold() open, so an upgrade
// meets a call in flight: '(callee)' -> '()'
IC_Async start_held_call_async() {
  IC_API ic_api(CanisterUpdate{"start_held_call"}, false);
  std::string callee_text;
  CandidArgs args;
  args.append(CandidTypePrincipal{&callee_text});
  CandidTypePrincipal callee = principal_arg(ic_api, args, callee_text);
  g_held_done = false;
  ic_api.to_wire();

  IC_CallResult<IC_CallReply> r =
      co_await IC_Call::unbounded_wait(callee, "hold");
  g_held_done = r.has_value();
}
void start_held_call() { start_held_call_async(); }

// A long computation, in chunks of at most `chunk_instructions`, with a
// self-yield after each, so the whole computation can exceed the
// instruction limit of one message. Every chunk loads its job from g_jobs
// into the shared g_work, and saves it back, before it yields.
// `reload = false` skips the load: a broken chunk, for the tests to prove
// that other calls run in between.
// '(job, seed, n, chunk_instructions, reload)' ->
// '(result, chunks, call_instructions, max_message_instructions)'
IC_Async compute_async() {
  IC_API ic_api(CanisterUpdate{"compute"}, false);
  uint32_t job{0};
  uint64_t seed{0}, n{0}, chunk_instructions{0};
  bool reload{true};
  CandidArgs args;
  args.append(CandidTypeNat32{&job});
  args.append(CandidTypeNat64{&seed});
  args.append(CandidTypeNat64{&n});
  args.append(CandidTypeNat64{&chunk_instructions});
  args.append(CandidTypeBool{&reload});
  ic_api.from_wire(args);

  g_jobs[job] = {seed, 0};
  g_work = g_jobs[job];
  uint32_t chunks{0};
  uint64_t max_message{0};
  while (true) {
    if (reload) g_work = g_jobs[job]; // load the job's saved state
    lcg_chunk(n, chunk_instructions);
    g_jobs[job] = g_work; // save it, BEFORE the co_await
    g_run_log.push_back(job);
    ++chunks;
    max_message = std::max(max_message,
                           IC_API::performance_counter(
                               IC_API::PerformanceCounterType::Instructions));
    if (g_jobs[job].i >= n) break;
    IC_CallResult<void> r = co_await ic_api.yield_fresh_budget();
    r.value();
  }
  CandidArgs out;
  out.append(CandidTypeNat64{g_jobs[job].x});
  out.append(CandidTypeNat32{chunks});
  out.append(CandidTypeNat64{IC_API::performance_counter(
      IC_API::PerformanceCounterType::CallContextInstructions)});
  out.append(CandidTypeNat64{max_message});
  ic_api.to_wire(out);
}
void compute() { compute_async(); }

// The same computation in one message, without yields: it traps when it
// exceeds the instruction limit. '(seed, n)' -> '(result)'
void compute_no_yield() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  uint64_t seed{0}, n{0};
  CandidArgs args;
  args.append(CandidTypeNat64{&seed});
  args.append(CandidTypeNat64{&n});
  ic_api.from_wire(args);
  g_work = {seed, 0};
  lcg_chunk(n, UINT64_MAX);
  ic_api.to_wire(CandidTypeNat64{g_work.x});
}

// The saved state of a job: '(job)' -> '(x, steps)'
void get_job() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  uint32_t job{0};
  ic_api.from_wire(CandidTypeNat32{&job});
  LcgState state = g_jobs[job];
  CandidArgs out;
  out.append(CandidTypeNat64{state.x});
  out.append(CandidTypeNat64{state.i});
  ic_api.to_wire(out);
}

// The job of every chunk that ran, in order: '()' -> '(vec nat32)'
void get_run_log() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.from_wire();
  ic_api.to_wire(CandidTypeVecNat32{g_run_log});
}

void clear_run_log() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  ic_api.from_wire();
  g_run_log.clear();
  ic_api.to_wire();
}

// '()' -> '(before, after, destroyed, calls_in_flight, held_done)'
void get_state() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.from_wire();
  CandidArgs out;
  out.append(CandidTypeNat64{g_before});
  out.append(CandidTypeNat64{g_after});
  out.append(CandidTypeNat64{g_destroyed});
  out.append(CandidTypeNat32{IC_API::calls_in_flight()});
  out.append(CandidTypeBool{g_held_done});
  ic_api.to_wire(out);
}
