// Inter-canister calls with C++20 coroutines - see ic_async.h.

#include "ic_api.h"

#if defined(__cpp_impl_coroutine)

#include <cstring>
#include <limits>

#include "candid_deserialize.h"
#include "candid_serialize.h"
#include "candid_serialize_type_table_registry.h"
#include "current_entry.h"
#include "entry_contexts.h"
#include "ic0.h"

// Gives the async machinery access to IC_API's private async_attach.
struct IC_AsyncAccess {
  static void attach(IC_API &ic_api, const CanisterBase &entry) {
    ic_api.async_attach(entry);
  }
};

namespace {
// The table index ic0.call_new takes for a call without callbacks: no valid
// function, so the response of a oneway call is dropped.
constexpr uintptr_t ONEWAY_FUN = std::numeric_limits<uint32_t>::max();

// The default and maximum timeout of a best-effort call.
constexpr uint32_t BOUNDED_WAIT_TIMEOUT_SECONDS = 300;

// A 128-bit value an ic0 function writes at dst. A template: the pointer type
// is uint32_t on wasm32 and uintptr_t in the Mock IC.
template <typename Fn> __uint128_t read_u128(Fn fn) {
  __uint128_t value{0};
  fn(reinterpret_cast<uintptr_t>(&value));
  return value;
}

std::string reject_message() {
  std::string message(ic0_msg_reject_msg_size(), '\0');
  ic0_msg_reject_msg_copy(reinterpret_cast<uintptr_t>(message.data()), 0,
                          (uint32_t)message.size());
  return message;
}

std::vector<uint8_t> arg_data() {
  std::vector<uint8_t> bytes(ic0_msg_arg_data_size());
  ic0_msg_arg_data_copy(reinterpret_cast<uintptr_t>(bytes.data()), 0,
                        (uint32_t)bytes.size());
  return bytes;
}

std::vector<uint8_t> canister_self_bytes() {
  std::vector<uint8_t> bytes(ic0_canister_self_size());
  ic0_canister_self_copy(reinterpret_cast<uintptr_t>(bytes.data()), 0,
                         (uint32_t)bytes.size());
  return bytes;
}

// An empty Candid argument list: ().
std::vector<uint8_t> empty_args() { return {'D', 'I', 'D', 'L', 0, 0}; }

std::vector<uint8_t> serialize(const CandidArgs &args) {
  VecBytes B = CandidSerialize(args).get_B();
  CandidSerializeTypeTableRegistry::get_instance().clear();
  std::vector<uint8_t> bytes;
  for (std::byte b : B.vec())
    bytes.push_back(static_cast<uint8_t>(b));
  return bytes;
}

void check_call_entry(const char *api) {
  CanisterBase *entry = current_entry();
  if (entry != nullptr && !entry_contexts::call(*entry)) {
    IC_API::trap(std::string(api) + " is not available in a " +
                 entry->get_entry_type() +
                 " entry point. Allowed entry points: update, composite "
                 "query, reply & reject callbacks, heartbeat & timer.");
  }
}

// ic0.call_new ... ic0.call_perform. Returns the call_perform result.
uint32_t perform(const std::vector<uint8_t> &callee, const std::string &method,
                 const std::vector<uint8_t> &args, __uint128_t cycles,
                 std::optional<uint32_t> timeout, uintptr_t fun, uintptr_t env,
                 uintptr_t cleanup_fun) {
  ic0_call_new(reinterpret_cast<uintptr_t>(callee.data()),
               (uint32_t)callee.size(),
               reinterpret_cast<uintptr_t>(method.data()),
               (uint32_t)method.size(), fun, env, fun, env);
  if (cleanup_fun != 0) ic0_call_on_cleanup(cleanup_fun, env);
  if (timeout.has_value()) ic0_call_with_best_effort_response(*timeout);
  ic0_call_data_append(reinterpret_cast<uintptr_t>(args.data()),
                       (uint32_t)args.size());
  if (cycles > 0) {
    ic0_call_cycles_add128((uint64_t)(cycles >> 64), (uint64_t)cycles);
  }
  return ic0_call_perform();
}

// The liquid balance must pay for the call and the cycles sent along.
std::optional<IC_CallError>
check_liquid_balance(const std::string &method,
                     const std::vector<uint8_t> &args, __uint128_t cycles) {
  __uint128_t cost{0};
  ic0_cost_call(method.size(), args.size(), reinterpret_cast<uintptr_t>(&cost));
  __uint128_t required = cost + cycles;
  __uint128_t available = read_u128(ic0_canister_liquid_cycle_balance128);
  if (available >= required) return std::nullopt;
  IC_CallError error;
  error.kind = IC_CallError::Kind::InsufficientLiquidCycleBalance;
  error.available = available;
  error.required = required;
  return error;
}
} // namespace

// ---------------------------------------------------------------------------
// The callbacks. One function serves as both the reply and the reject
// callback: ic0.msg_reject_code() tells them apart. The env is the
// IC_CallSlot of the call.

namespace {
// Clears what a callback set up, before it returns.
void end_callback() {
  clear_current_entry();
  set_live_ic_api(nullptr);
}
} // namespace

extern "C" void icpp_call_callback(uintptr_t env) {
  IC_CallSlot *slot = reinterpret_cast<IC_CallSlot *>(env);
  IC_CallsState *calls = slot->calls;
  if (slot->counted) {
    slot->counted = false;
    add_calls_in_flight(-1);
  }

  uint32_t code = ic0_msg_reject_code();
  CanisterBase entry =
      calls->composite
          ? (code == 0
                 ? CanisterBase(CanisterCompositeReplyCallback{slot->method})
                 : CanisterBase(CanisterCompositeRejectCallback{slot->method}))
          : (code == 0 ? CanisterBase(CanisterReplyCallback{slot->method})
                       : CanisterBase(CanisterRejectCallback{slot->method}));
  set_current_entry(entry);
  CandidSerializeTypeTableRegistry::get_instance().clear();

  // A composite query sends no cycles, and can't read refunded cycles
  __uint128_t refunded =
      calls->composite ? 0 : read_u128(ic0_msg_cycles_refunded128);
  if (code == 0) {
    std::vector<uint8_t> bytes = arg_data();
    calls->replies[slot->index].bytes.store(bytes.data(),
                                            (uint32_t)bytes.size());
    calls->replies[slot->index].cycles_refunded = refunded;
  } else {
    IC_CallError error;
    error.kind = IC_CallError::Kind::CallRejected;
    error.reject_code = static_cast<IC_RejectCode>(code);
    error.reject_message = reject_message();
    error.cycles_refunded = refunded;
    calls->errors[slot->index] = error;
  }
  --calls->pending;
  slot->delivered = true;

  // Not the last response: this message only stores it
  if (calls->pending > 0) {
    delete slot;
    end_callback();
    return;
  }

  // Re-attach the IC_API of the coroutine, so its guards see this callback
  if (calls->ic_api != nullptr && is_registered_ic_api(calls->ic_api)) {
    IC_AsyncAccess::attach(*calls->ic_api, entry);
  } else {
    set_live_ic_api(nullptr);
  }

  // The coroutine may finish here and free its frame. The state is not in
  // the frame: delete it once the coroutine no longer needs it. If the code
  // after the co_await traps, these lines are never reached, and the cleanup
  // callback deletes the state instead.
  calls->handle.resume();
  delete calls;
  delete slot;
  end_callback();
}

// Runs only when a callback trapped. On the IC, the trap rolled back
// everything the callback did; in the Mock IC, which has no rollback, the
// callback may have stored its response first - slot->delivered tells.
// - The last response trapped - the code after the co_await: the coroutines
//   are suspended at their co_awaits again. Destroy the root of the chain,
//   which destroys the tasks it awaits, to free their frames.
// - Another response trapped, before any code of the coroutine ran: record
//   the error; a later response resumes the coroutine.
extern "C" void icpp_call_cleanup(uintptr_t env) {
  IC_CallSlot *slot = reinterpret_cast<IC_CallSlot *>(env);
  IC_CallsState *calls = slot->calls;
  if (slot->counted) {
    slot->counted = false;
    add_calls_in_flight(-1);
  }
  if (!slot->delivered) {
    --calls->pending;
    IC_CallError error;
    error.kind = IC_CallError::Kind::CallRejected;
    error.reject_code = IC_RejectCode::SysUnknown;
    error.reject_message = "the callback of this call trapped";
    calls->errors[slot->index] = error;
  }

  CanisterBase entry =
      calls->composite
          ? CanisterBase(CanisterCompositeCleanupCallback{slot->method})
          : CanisterBase(CanisterCleanupCallback{slot->method});
  set_current_entry(entry);
  if (calls->pending == 0) {
    if (calls->ic_api != nullptr && is_registered_ic_api(calls->ic_api)) {
      IC_AsyncAccess::attach(*calls->ic_api, entry);
    }
    calls->root.destroy();
    delete calls;
  }
  delete slot;
  end_callback();
}

// ---------------------------------------------------------------------------
// IC_Call

IC_Call IC_Call::make(const CandidTypePrincipal &callee,
                      const std::string &method,
                      std::optional<uint32_t> timeout_seconds) {
  IC_Call call;
  call.m_callee = callee.get_v_bytes().vec_uint8_t();
  call.m_method = method;
  call.m_args = empty_args();
  call.m_timeout_seconds = timeout_seconds;
  return call;
}

IC_Call IC_Call::bounded_wait(const CandidTypePrincipal &callee,
                              const std::string &method) {
  return make(callee, method, BOUNDED_WAIT_TIMEOUT_SECONDS);
}

IC_Call IC_Call::unbounded_wait(const CandidTypePrincipal &callee,
                                const std::string &method) {
  return make(callee, method, std::nullopt);
}

IC_Call IC_Call::guaranteed_response(const CandidTypePrincipal &callee,
                                     const std::string &method) {
  return make(callee, method, std::nullopt);
}

IC_Call &IC_Call::with_arg(CandidType arg) {
  CandidArgs args;
  args.append(arg);
  return with_args(args);
}

IC_Call &IC_Call::with_args(CandidArgs args) {
  m_args = serialize(args);
  return *this;
}

IC_Call &IC_Call::with_raw_args(const std::vector<uint8_t> &bytes) {
  m_args = bytes;
  return *this;
}

IC_Call &IC_Call::with_cycles(__uint128_t cycles) {
  m_cycles = cycles;
  return *this;
}

IC_Call &IC_Call::change_timeout(uint32_t seconds) {
  if (!m_timeout_seconds.has_value()) {
    IC_API::trap("IC_Call::change_timeout: only a bounded_wait call has a "
                 "timeout");
  }
  m_timeout_seconds = seconds;
  return *this;
}

IC_CallResult<void> IC_Call::oneway() {
  check_call_entry("IC_Call::oneway");
  if (auto error = check_liquid_balance(m_method, m_args, m_cycles)) {
    return *error;
  }
  uint32_t rc = perform(m_callee, m_method, m_args, m_cycles, m_timeout_seconds,
                        ONEWAY_FUN, 0, 0);
  if (rc != 0) {
    IC_CallError error;
    error.kind = IC_CallError::Kind::CallPerformFailed;
    error.perform_code = rc;
    return error;
  }
  return {};
}

IC_CallAwaiter IC_Call::operator co_await() const {
  return IC_CallAwaiter(*this);
}

// ---------------------------------------------------------------------------
// IC_CallsAwaiter, IC_CallAll

IC_CallsAwaiter::~IC_CallsAwaiter() {
  if (!m_performed) delete m_state;
}

bool IC_CallsAwaiter::suspend(std::coroutine_handle<> handle,
                              std::coroutine_handle<> root) {
  check_call_entry("IC_Call");
  CanisterBase *entry = current_entry();
  m_state->composite = entry != nullptr && entry->is_composite();
  m_state->handle = handle;
  m_state->root = root;
  m_state->ic_api = live_ic_api();
  m_state->errors.assign(m_calls.size(), std::nullopt);
  m_state->replies.assign(m_calls.size(), IC_CallReply{});

  for (uint32_t i = 0; i < m_calls.size(); ++i) {
    const IC_Call &call = m_calls[i];
    if (m_state->composite && call.m_cycles > 0) {
      IC_API::trap("IC_Call::with_cycles: a composite query can't send "
                   "cycles along with a call");
    }
    // A call that is not performed gets its error right away: no callback
    // will ever run for it. The balance is checked per call: each one
    // performed lowers it.
    if (auto error =
            check_liquid_balance(call.m_method, call.m_args, call.m_cycles)) {
      m_state->errors[i] = error;
      continue;
    }
    IC_CallSlot *slot = new IC_CallSlot{m_state, i, call.m_method};
    uint32_t rc = perform(call.m_callee, call.m_method, call.m_args,
                          call.m_cycles, call.m_timeout_seconds,
                          reinterpret_cast<uintptr_t>(&icpp_call_callback),
                          reinterpret_cast<uintptr_t>(slot),
                          reinterpret_cast<uintptr_t>(&icpp_call_cleanup));
    if (rc != 0) {
      delete slot;
      IC_CallError error;
      error.kind = IC_CallError::Kind::CallPerformFailed;
      error.perform_code = rc;
      m_state->errors[i] = error;
      continue;
    }
    // A composite query's calls are not counted: the IC discards the state of
    // a query, and they can't block stopping or upgrading the canister.
    if (!m_state->composite) {
      slot->counted = true;
      add_calls_in_flight(1);
    }
    ++m_state->pending;
  }

  // No call performed: resume right away, with the errors
  if (m_state->pending == 0) return false;

  // Suspend: the entry function returns, which commits this message. The
  // IC_API stays in the coroutine frame, but is not live in other messages.
  m_performed = true;
  clear_current_entry();
  set_live_ic_api(nullptr);
  return true;
}

IC_CallResult<IC_CallReply> IC_CallsAwaiter::result(size_t i) {
  if (m_state->errors[i].has_value()) return *m_state->errors[i];
  return std::move(m_state->replies[i]);
}

IC_CallAll::Awaiter IC_CallAll::operator co_await() && {
  return Awaiter(std::move(m_calls));
}

std::vector<IC_CallResult<IC_CallReply>> IC_CallAll::Awaiter::await_resume() {
  std::vector<IC_CallResult<IC_CallReply>> results;
  results.reserve(size());
  for (size_t i = 0; i < size(); ++i)
    results.push_back(result(i));
  return results;
}

// ---------------------------------------------------------------------------
// IC_Yield

IC_Yield::IC_Yield(IC_Call call) : m_call(std::move(call)) {}

IC_Yield::Awaiter IC_Yield::operator co_await() const {
  return Awaiter(m_call);
}

bool IC_Yield::Awaiter::stopping() {
  return ic0_canister_status() !=
         static_cast<uint32_t>(IC_API::CanisterStatus::Running);
}

IC_CallResult<void> IC_Yield::Awaiter::await_resume() {
  IC_CallError error;
  error.kind = IC_CallError::Kind::CanisterStopping;
  if (m_stopping) return error;

  IC_CallResult<IC_CallReply> result = this->result(0);
  if (result) return {};
  if (result.error().kind != IC_CallError::Kind::CallRejected) {
    return result.error();
  }
  // The stop began while the self-call was in flight
  if (stopping()) {
    error.reject_code = result.error().reject_code;
    error.reject_message = result.error().reject_message;
    return error;
  }
  // Any other reject - e.g. SysUnknown or SysTransient - also continues in a
  // new message: the loop can keep working.
  return {};
}

IC_Yield IC_API::yield_fresh_budget() {
  return IC_Yield(IC_Call::bounded_wait(m_canister_self, "__icpp_yield"));
}

// The method yield_fresh_budget calls. Only the canister itself may call it.
void icpp_yield() WASM_SYMBOL_EXPORTED("canister_update __icpp_yield");
void icpp_yield() {
  std::vector<uint8_t> caller(ic0_msg_caller_size());
  ic0_msg_caller_copy(reinterpret_cast<uintptr_t>(caller.data()), 0,
                      (uint32_t)caller.size());
  if (caller != canister_self_bytes()) {
    std::string message = "__icpp_yield: only the canister itself may call it";
    ic0_msg_reject(reinterpret_cast<uintptr_t>(message.data()),
                   (uint32_t)message.size());
    return;
  }
  std::vector<uint8_t> reply = empty_args();
  ic0_msg_reply_data_append(reinterpret_cast<uintptr_t>(reply.data()),
                            (uint32_t)reply.size());
  ic0_msg_reply();
}

// A canister that makes calls refuses to upgrade while calls are in flight.
// Weak: a canister_pre_upgrade of the canister itself takes precedence, and
// its IC_API(CanisterPreUpgrade{...}) runs the same check.
__attribute__((weak)) void icpp_default_pre_upgrade()
    WASM_SYMBOL_EXPORTED("canister_pre_upgrade");
__attribute__((weak)) void icpp_default_pre_upgrade() {
  icpp_check_no_calls_in_flight();
}

// ---------------------------------------------------------------------------
// IC_Async, IC_CallResult, IC_CallReply, IC_CallError

void icpp_unhandled_exception() {
#if defined(__cpp_exceptions) && !defined(__wasm__)
  throw; // The Mock IC: a trap is a C++ exception
#else
  IC_API::trap("IC_Async / IC_Task: unhandled exception");
#endif
}

void icpp_call_error_trap(const IC_CallError &error) {
  IC_API::trap("IC_CallResult::value(): " + error.to_string());
}

void IC_CallResult<void>::value() const {
  if (m_e.has_value()) icpp_call_error_trap(*m_e);
}

const IC_CallError &IC_CallResult<void>::error() const {
  if (!m_e.has_value()) {
    IC_API::trap("IC_CallResult::error(): the call succeeded");
  }
  return *m_e;
}

void IC_CallReply::decode() { decode(CandidArgs{}); }

void IC_CallReply::decode(CandidType arg) {
  CandidArgs args;
  args.append(arg);
  decode(args);
}

void IC_CallReply::decode(CandidArgs args) {
  CandidDeserialize(bytes, args);
  CandidSerializeTypeTableRegistry::get_instance().clear();
}

bool IC_CallError::is_clean_reject() const {
  return kind == Kind::CallRejected &&
         (reject_code == IC_RejectCode::SysFatal ||
          reject_code == IC_RejectCode::SysTransient ||
          reject_code == IC_RejectCode::DestinationInvalid);
}

bool IC_CallError::is_immediately_retryable() const {
  return kind == Kind::CallRejected &&
         (reject_code == IC_RejectCode::SysTransient ||
          reject_code == IC_RejectCode::SysUnknown);
}

std::string IC_CallError::to_string() const {
  switch (kind) {
  case Kind::InsufficientLiquidCycleBalance:
    return "insufficient liquid cycle balance: " +
           IC_API::to_string_128(available) + " available, " +
           IC_API::to_string_128(required) + " required";
  case Kind::CallPerformFailed:
    return "ic0.call_perform failed with code " + std::to_string(perform_code);
  case Kind::SignCostFailed:
    return sign_cost_error == IC_API::SignCostError::InvalidKeyName
               ? "unknown key name"
               : "unknown curve or algorithm";
  case Kind::CanisterStopping:
    return "the canister is stopping";
  case Kind::CallRejected:
    break;
  }
  return "call rejected with code " +
         std::to_string(static_cast<uint32_t>(reject_code)) + ": " +
         reject_message;
}

#endif // __cpp_impl_coroutine
