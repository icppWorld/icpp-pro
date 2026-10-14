// Inter-canister calls (ic_async.h): the callee methods, and the caller
// methods that call them - on another canister, or on the canister itself.

#include "my_canister.h"

#include <string>
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

// Replies once release() was called, or after 60 s: '()' -> '()'
IC_Async hold_async() {
  IC_API ic_api(CanisterUpdate{"hold"}, false);
  ic_api.from_wire();
  g_released = false;
  uint64_t until = IC_API::time() + 60'000'000'000;
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
