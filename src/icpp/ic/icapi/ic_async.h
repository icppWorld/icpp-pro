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
// limit. ic_api.get_caller() is still the original caller.
//
// The mechanics follow the Rust CDK (ic-cdk): one callback function for both
// reply and reject, with the call state as its env, and a cleanup callback
// that frees the suspended coroutine when the code after a co_await traps.
//
// Not available in C++17 builds, which have no coroutines.

#pragma once

#if defined(__cpp_impl_coroutine)

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
    CallRejected
  };
  Kind kind{Kind::CallRejected};
  IC_RejectCode reject_code{IC_RejectCode::NoError}; // CallRejected
  std::string reject_message;                        // CallRejected
  __uint128_t cycles_refunded{0};                    // CallRejected
  __uint128_t available{0}; // InsufficientLiquidCycleBalance
  __uint128_t required{0};  // InsufficientLiquidCycleBalance
  uint32_t perform_code{0}; // CallPerformFailed

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
  static IC_Call unbounded_wait(const CandidTypePrincipal &callee,
                                const std::string &method);

  // The argument. Without one, the call sends no arguments: ().
  IC_Call &with_arg(CandidType arg);
  IC_Call &with_args(CandidArgs args);
  IC_Call &with_raw_args(const std::vector<uint8_t> &bytes);
  // Cycles to send along. The callee accepts what it wants; the rest comes
  // back as cycles_refunded.
  IC_Call &with_cycles(__uint128_t cycles);
  // Bounded-wait calls only.
  IC_Call &change_timeout(uint32_t seconds);

  // Performs the call without waiting for, or receiving, a response.
  IC_CallResult<void> oneway();

  IC_CallAwaiter operator co_await() const;

private:
  friend class IC_CallAwaiter;
  std::vector<uint8_t> m_callee;
  std::string m_method;
  std::vector<uint8_t> m_args;
  __uint128_t m_cycles{0};
  std::optional<uint32_t> m_timeout_seconds;
};

// The return type of a coroutine that makes calls. It starts right away and
// runs until its first co_await; the rest runs in the callbacks.
class IC_Async {
public:
  struct promise_type {
    IC_Async get_return_object() noexcept { return {}; }
    std::suspend_never initial_suspend() noexcept { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }
    void return_void() noexcept {}
    void unhandled_exception();
  };
};

// co_await ic_api.yield_fresh_budget() continues in a new message - the
// reply or reject callback of a call to the canister itself - with a fresh
// instruction limit. An error means no new message started.
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

// The state of one awaited call. It lives in the coroutine frame, and its
// address is the env of the callbacks.
struct IC_CallState {
  std::coroutine_handle<> handle;
  IC_API *api{nullptr};
  std::string method;
  bool counted{false}; // counted in calls_in_flight
  std::optional<IC_CallError> error;
  IC_CallReply reply;
};

class IC_CallAwaiter {
public:
  explicit IC_CallAwaiter(IC_Call call) : m_call(std::move(call)) {}
  bool await_ready() const noexcept { return false; }
  bool await_suspend(std::coroutine_handle<> handle);
  IC_CallResult<IC_CallReply> await_resume();

private:
  IC_Call m_call;
  IC_CallState m_state;
};

class IC_Yield::Awaiter {
public:
  explicit Awaiter(IC_Call call) : m_inner(std::move(call)) {}
  bool await_ready() const noexcept { return false; }
  bool await_suspend(std::coroutine_handle<> handle) {
    return m_inner.await_suspend(handle);
  }
  IC_CallResult<void> await_resume();

private:
  IC_CallAwaiter m_inner;
};

[[noreturn]] void icpp_call_error_trap(const IC_CallError &error);

template <typename T> T &IC_CallResult<T>::value() {
  if (!has_value()) icpp_call_error_trap(std::get<1>(m_v));
  return std::get<0>(m_v);
}

template <typename T> const IC_CallError &IC_CallResult<T>::error() const {
  if (has_value()) IC_API::trap("IC_CallResult::error(): the call succeeded");
  return std::get<1>(m_v);
}

#endif // __cpp_impl_coroutine
