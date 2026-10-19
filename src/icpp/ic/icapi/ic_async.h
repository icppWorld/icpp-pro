// Inter-canister calls with C++20 coroutines.
//
// An update method that makes calls is written as a coroutine returning
// IC_Async, started from the exported () -> () entry point:
//
//   IC_Async greet_async() {
//     IC_API ic_api(CanisterUpdate{"greet"}, false);
//     IC_CallResult<IC_CallReply> r =
//         co_await IC_Call::bounded_wait(callee, "hello")
//             .with_arg(CandidTypeText{"world"});
//     if (!r) { ... r.error() ... }
//     std::string s;
//     r->decode(CandidTypeText{&s});
//     ic_api.to_wire(CandidTypeText{s}); // replies to the original caller
//   }
//   void greet() { greet_async(); }
//
// Each co_await is a commit point: the state changes made before it are
// kept, even when the code after it traps. The code after a co_await runs in
// the reply or reject callback - a new message, with its own instruction
// limit: 38B instructions, where the first message of an update gets 40B,
// because the IC reserves 5% for the cleanup callback.
// ic_api.get_caller() is still the original caller.
//
// A trap after a co_await rolls back only that message. State set before a
// commit point stays, so a flag like busy[key] = true, set before the first
// co_await, stays set forever when a later message traps. An RAII guard does
// not help: the cleanup destroys the coroutine frame without running its
// destructors. Give such an entry a time-to-live, e.g. store its start time
// and treat it as expired after a while.
//
// The mechanics follow the Rust CDK (ic-cdk): one callback function for both
// reply and reject, with the call state as its env, and a cleanup callback
// that frees the suspended coroutines when the code after a co_await traps.
//
// A coroutine can co_await another coroutine that returns IC_Task<T>, and get
// its result; IC_CallAll makes several calls at once.
//
// Not available in C++17 builds, which have no coroutines.

#pragma once

#if defined(__cpp_impl_coroutine)

#include <concepts>
#include <coroutine>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

// docs start: ic_async
// The reject codes of the IC.
enum class IC_RejectCode : uint32_t {
  NoError = 0,
  SysFatal = 1,
  SysTransient = 2,
  DestinationInvalid = 3,
  CanisterReject = 4,
  CanisterError = 5,
  SysUnknown = 6 // a best-effort call timed out, or its response was lost
};

// Why a call did not produce a reply.
struct IC_CallError {
  enum class Kind : uint32_t {
    // Not performed: the liquid balance can't pay for the call
    InsufficientLiquidCycleBalance,
    // Not performed: ic0.call_perform failed
    CallPerformFailed,
    // Performed, and rejected by the callee or the IC
    CallRejected,
    // Not performed: the cost of a signature or vetKD key is unknown - the
    // key name or the curve/algorithm is not known (IC_Management)
    SignCostFailed,
    // yield_fresh_budget only: the canister is stopping. Reply now with what
    // you have
    CanisterStopping
  };
  Kind kind{Kind::CallRejected};
  IC_RejectCode reject_code{IC_RejectCode::NoError}; // CallRejected
  std::string reject_message;                        // CallRejected
  __uint128_t cycles_refunded{0};                    // CallRejected
  __uint128_t available{0}; // InsufficientLiquidCycleBalance
  __uint128_t required{0};  // InsufficientLiquidCycleBalance
  uint32_t perform_code{0}; // CallPerformFailed
  IC_API::SignCostError sign_cost_error{IC_API::SignCostError::None};

  // The callee did not run the call: retrying can't do it twice.
  bool is_clean_reject() const;
  // A retry may succeed right away.
  bool is_immediately_retryable() const;
  std::string to_string() const;
};

// The reply of a call.
struct IC_CallReply {
  VecBytes bytes;
  __uint128_t cycles_refunded{0};

  // Decode the Candid reply, like ic_api.from_wire(). Traps on a mismatch.
  void decode();
  void decode(CandidType arg);
  void decode(CandidArgs args);
};

// A value or an IC_CallError, shaped like C++23's std::expected.
template <typename T> class [[nodiscard]] IC_CallResult {
public:
  IC_CallResult(T value) : m_v(std::move(value)) {}
  IC_CallResult(IC_CallError error) : m_v(std::move(error)) {}

  bool has_value() const { return m_v.index() == 0; }
  explicit operator bool() const { return has_value(); }

  // Traps when the call failed: handle error() first.
  T &value();
  T *operator->() { return &value(); }
  T &operator*() { return value(); }

  // Traps when the call succeeded.
  const IC_CallError &error() const;

private:
  std::variant<T, IC_CallError> m_v;
};

template <> class [[nodiscard]] IC_CallResult<void> {
public:
  IC_CallResult() = default;
  IC_CallResult(IC_CallError error) : m_e(std::move(error)) {}

  bool has_value() const { return !m_e.has_value(); }
  explicit operator bool() const { return has_value(); }
  void value() const; // Traps when the call failed
  const IC_CallError &error() const;

private:
  std::optional<IC_CallError> m_e;
};

class IC_CallAwaiter;
class IC_CallsAwaiter;

#if defined(ICPP_ALLOW_UNBOUNDED_WAIT)
#define ICPP_UNBOUNDED_WAIT_WARNING
#else
#define ICPP_UNBOUNDED_WAIT_WARNING                                            \
  __attribute__((diagnose_if(                                                  \
      true,                                                                    \
      "IC_Call::unbounded_wait: while this call is in flight, the canister "   \
      "can't be stopped or upgraded, and a callee that never responds blocks " \
      "that forever. Use IC_Call::bounded_wait unless you trust the callee. "  \
      "To turn off this warning, add \"-DICPP_ALLOW_UNBOUNDED_WAIT\" to "      \
      "cpp_compile_flags in both the [build-wasm] and [build-native] "         \
      "sections of icpp.toml.",                                                \
      "warning")))
#endif

// A call to a method of a canister, possibly the canister itself.
class IC_Call {
public:
  // A best-effort call: the IC rejects it with SysUnknown when no response
  // arrived within the timeout (default and maximum 300 s). Use it unless
  // the response must never be lost.
  static IC_Call bounded_wait(const CandidTypePrincipal &callee,
                              const std::string &method);
  // A guaranteed-response call: the reply or reject always arrives, however
  // long the callee takes.
  // WARNING: while it is in flight, the canister can't be stopped, so it can't
  // be upgraded. A callee that never responds blocks that forever. Use it only
  // for a callee you trust. Calling it gives a compiler warning, unless
  // ICPP_ALLOW_UNBOUNDED_WAIT is defined.
  static IC_Call
  unbounded_wait(const CandidTypePrincipal &callee,
                 const std::string &method) ICPP_UNBOUNDED_WAIT_WARNING;

  // The argument. Without one, the call sends no arguments: ().
  IC_Call &with_arg(CandidType arg);
  IC_Call &with_args(CandidArgs args);
  IC_Call &with_raw_args(const std::vector<uint8_t> &bytes);
  // Cycles to send along. The callee accepts what it wants; the rest comes
  // back as cycles_refunded.
  IC_Call &with_cycles(__uint128_t cycles);
  // Bounded-wait calls only. The IC caps the timeout at 300 s. A timeout of
  // 0 is valid, and almost always ends in a SysUnknown reject.
  IC_Call &change_timeout(uint32_t seconds);

  // Performs the call without waiting for, or receiving, a response.
  IC_CallResult<void> oneway();

  IC_CallAwaiter operator co_await() const;

private:
  friend class IC_CallsAwaiter;
  friend class IC_Management;
  // A guaranteed-response call to a trusted callee, without the warning of
  // unbounded_wait: for the management canister helpers
  static IC_Call guaranteed_response(const CandidTypePrincipal &callee,
                                     const std::string &method);
  // Every IC_Call comes from bounded_wait or unbounded_wait
  IC_Call() = default;
  static IC_Call make(const CandidTypePrincipal &callee,
                      const std::string &method,
                      std::optional<uint32_t> timeout_seconds);
  std::vector<uint8_t> m_callee;
  std::string m_method;
  std::vector<uint8_t> m_args;
  __uint128_t m_cycles{0};
  std::optional<uint32_t> m_timeout_seconds;
};

// Several calls at once: they are all performed in this message, and the
// co_await resumes once every response arrived, with the results in the
// order of the calls.
//   std::vector<IC_CallResult<IC_CallReply>> results =
//       co_await IC_CallAll({call_1, call_2, call_3});
class IC_CallAll {
public:
  explicit IC_CallAll(std::vector<IC_Call> calls) : m_calls(std::move(calls)) {}
  class Awaiter;
  Awaiter operator co_await() &&;

private:
  std::vector<IC_Call> m_calls;
};

// The return type of a coroutine that makes calls, started from the exported
// method. It starts right away and runs until its first co_await; the rest
// runs in the callbacks.
class IC_Async {
public:
  struct promise_type;
};

// The return type of a coroutine that another coroutine co_awaits, to get its
// result. It starts when it is awaited - a task that is never awaited never
// runs - and the awaiting coroutine continues when it finishes:
//   IC_Task<std::string> greet_task(CandidTypePrincipal callee) {
//     IC_CallResult<IC_CallReply> r =
//         co_await IC_Call::bounded_wait(callee, "greet");
//     std::string s;
//     r->decode(CandidTypeText{&s});
//     co_return s;
//   }
//   ... std::string s = co_await greet_task(callee);
// Only the coroutine started from the exported method constructs an IC_API;
// a task gets what it needs as arguments. Pass them by value: a task runs
// across messages, after the caller's temporaries are gone.
template <typename T = void> class IC_Task;

// co_await ic_api.yield_fresh_budget() continues in a new message - the
// reply or reject callback of a call to the canister itself - with a fresh
// instruction limit of 38B. The result:
// - success: you are in a new message, with a fresh instruction limit.
// - CanisterStopping: the canister is stopping. Reply now with what you have,
//   and do no further heavy work: the limit may not be fresh.
// - any other error: the call was not performed, no new message started.
//   IC_CallResult<void> r = co_await ic_api.yield_fresh_budget();
//   if (!r) { /* reply with the partial result */ co_return; }
class IC_Yield {
public:
  explicit IC_Yield(IC_Call call);
  class Awaiter;
  Awaiter operator co_await() const;

private:
  IC_Call m_call;
}; // docs end: ic_async

// ---------------------------------------------------------------------------
// Implementation details

[[noreturn]] void icpp_call_error_trap(const IC_CallError &error);
// Rethrows in the Mock IC, where a trap is a C++ exception; traps on the IC.
void icpp_unhandled_exception();

// Every coroutine of a chain - the IC_Async started from the exported method,
// and the IC_Tasks it awaits - knows the root of the chain: the IC_Async. A
// call resumes the innermost coroutine; after a trap, the cleanup destroys
// the root, whose destructor destroys the tasks it awaits, and so on.
struct IC_PromiseBase {
  std::coroutine_handle<> root;         // the IC_Async of the chain
  std::coroutine_handle<> continuation; // the coroutine awaiting this task
};
template <typename P>
concept IC_Promise = std::derived_from<P, IC_PromiseBase>;

struct IC_Async::promise_type : IC_PromiseBase {
  IC_Async get_return_object() noexcept {
    root = std::coroutine_handle<promise_type>::from_promise(*this);
    return {};
  }
  std::suspend_never initial_suspend() noexcept { return {}; }
  std::suspend_never final_suspend() noexcept { return {}; }
  void return_void() noexcept {}
  void unhandled_exception() { icpp_unhandled_exception(); }
};

// A finished task continues the coroutine that awaits it.
struct IC_TaskFinalAwaiter {
  bool await_ready() const noexcept { return false; }
  template <IC_Promise P>
  std::coroutine_handle<> await_suspend(std::coroutine_handle<P> h) noexcept {
    return h.promise().continuation;
  }
  void await_resume() const noexcept {}
};

template <typename T> struct IC_TaskPromise : IC_PromiseBase {
  std::optional<T> value;
  template <typename U> void return_value(U &&v) {
    value.emplace(std::forward<U>(v));
  }
  T result() { return std::move(*value); }
};
template <> struct IC_TaskPromise<void> : IC_PromiseBase {
  void return_void() noexcept {}
  void result() {}
};

template <typename T> class [[nodiscard]] IC_Task {
public:
  struct promise_type : IC_TaskPromise<T> {
    IC_Task get_return_object() noexcept {
      return IC_Task(std::coroutine_handle<promise_type>::from_promise(*this));
    }
    std::suspend_always initial_suspend() noexcept { return {}; }
    IC_TaskFinalAwaiter final_suspend() noexcept { return {}; }
    void unhandled_exception() { icpp_unhandled_exception(); }
  };

  IC_Task(IC_Task &&other) noexcept : m_h(other.m_h) { other.m_h = {}; }
  IC_Task &operator=(IC_Task &&other) noexcept {
    if (this != &other) {
      if (m_h) m_h.destroy();
      m_h = other.m_h;
      other.m_h = {};
    }
    return *this;
  }
  IC_Task(const IC_Task &) = delete;
  IC_Task &operator=(const IC_Task &) = delete;
  // Destroys the task's frame: a finished task, or - after a trap - a task
  // suspended at a co_await, as part of the chain the cleanup destroys.
  ~IC_Task() {
    if (m_h) m_h.destroy();
  }

  class Awaiter {
  public:
    explicit Awaiter(std::coroutine_handle<promise_type> h) : m_h(h) {}
    bool await_ready() const noexcept { return false; }
    // Starts the task, in the chain of the awaiting coroutine
    template <IC_Promise P>
    std::coroutine_handle<> await_suspend(std::coroutine_handle<P> parent) {
      m_h.promise().root = parent.promise().root;
      m_h.promise().continuation = parent;
      return m_h;
    }
    T await_resume() { return m_h.promise().result(); }

  private:
    std::coroutine_handle<promise_type> m_h;
  };

  // A task is awaited once: co_await my_task(...), or co_await std::move(t).
  Awaiter operator co_await() && {
    if (!m_h || m_h.promise().continuation) {
      IC_API::trap("IC_Task: a task can be awaited only once");
    }
    return Awaiter(m_h);
  }

private:
  explicit IC_Task(std::coroutine_handle<promise_type> h) : m_h(h) {}
  std::coroutine_handle<promise_type> m_h;
};

// The calls of one co_await. Each call's env is an IC_CallSlot; the state is
// shared. Both live on the heap: once a call is performed, they belong to the
// callbacks, which outlive the awaiter.
struct IC_CallsState {
  std::coroutine_handle<> handle; // resumed by the last response
  std::coroutine_handle<> root;   // destroyed by the cleanup after a trap
  IC_API *ic_api{nullptr};
  bool composite{false}; // made from a composite query
  uint32_t pending{0};   // calls whose response did not arrive yet
  std::vector<std::optional<IC_CallError>> errors;
  std::vector<IC_CallReply> replies;
};
struct IC_CallSlot {
  IC_CallsState *calls;
  uint32_t index;
  std::string method;
  bool counted{false};   // counted in calls_in_flight
  bool delivered{false}; // the callback stored its response
};

// Performs the calls of one co_await, and suspends until every response
// arrived. A call that can't be performed gets its error right away; when no
// call was performed, the coroutine does not suspend.
class IC_CallsAwaiter {
public:
  explicit IC_CallsAwaiter(std::vector<IC_Call> calls)
      : m_calls(std::move(calls)) {}
  IC_CallsAwaiter(const IC_CallsAwaiter &) = delete;
  IC_CallsAwaiter &operator=(const IC_CallsAwaiter &) = delete;
  ~IC_CallsAwaiter();
  bool await_ready() const noexcept { return false; }
  template <IC_Promise P> bool await_suspend(std::coroutine_handle<P> handle) {
    return suspend(handle, handle.promise().root);
  }

protected:
  bool suspend(std::coroutine_handle<> handle, std::coroutine_handle<> root);
  IC_CallResult<IC_CallReply> result(size_t i);
  size_t size() const { return m_calls.size(); }

private:
  std::vector<IC_Call> m_calls;
  IC_CallsState *m_state{new IC_CallsState};
  bool m_performed{false}; // the callbacks own m_state
};

class IC_CallAwaiter : public IC_CallsAwaiter {
public:
  explicit IC_CallAwaiter(IC_Call call)
      : IC_CallsAwaiter(std::vector<IC_Call>{std::move(call)}) {}
  IC_CallResult<IC_CallReply> await_resume() { return result(0); }
};

class IC_CallAll::Awaiter : public IC_CallsAwaiter {
public:
  explicit Awaiter(std::vector<IC_Call> calls)
      : IC_CallsAwaiter(std::move(calls)) {}
  std::vector<IC_CallResult<IC_CallReply>> await_resume();
};

class IC_Yield::Awaiter : public IC_CallsAwaiter {
public:
  explicit Awaiter(IC_Call call)
      : IC_CallsAwaiter(std::vector<IC_Call>{std::move(call)}) {}
  // A stopping canister does not call itself: the IC would reject the call
  template <IC_Promise P> bool await_suspend(std::coroutine_handle<P> handle) {
    m_stopping = stopping();
    return !m_stopping && suspend(handle, handle.promise().root);
  }
  IC_CallResult<void> await_resume();

private:
  static bool stopping();
  bool m_stopping{false};
};

template <typename T> T &IC_CallResult<T>::value() {
  if (!has_value()) icpp_call_error_trap(std::get<1>(m_v));
  return std::get<0>(m_v);
}

template <typename T> const IC_CallError &IC_CallResult<T>::error() const {
  if (has_value()) IC_API::trap("IC_CallResult::error(): the call succeeded");
  return std::get<1>(m_v);
}

#endif // __cpp_impl_coroutine
