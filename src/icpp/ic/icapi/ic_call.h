// API for performing inter-canister calls from a canister update or
// reply/reject callback context.
//
// Two layers are provided:
//
//   1. Low-level `IC_Call::raw_call(...)` — encodes a Candid payload, fires
//      ic0.call_new + ic0.call_data_append + ic0.call_perform, and registers a
//      C++ callback (reply + reject) indexed by an internal env cookie. The
//      caller's update message returns immediately; the IC will later invoke
//      the registered callback on this canister with the callee's reply bytes
//      or reject reason.
//
//   2. High-level `IC_CallBuilder` — fluent wrapper around `raw_call` that
//      accepts a target principal (textual or bytes), a method name, a Candid
//      argument list, and `on_reply` / `on_reject` lambdas.
//
// Both layers persist the active callback table in a static registry whose
// lifetime is the canister's WASM heap, not the message. The registry uses a
// monotonically increasing env id and is cleared when the canister upgrades.
// Callers that need their callback state to survive an upgrade must persist
// it through `pre_upgrade` / `post_upgrade` just like any other heap data.
//
// Failure model:
//   * If `ic0.call_perform` returns non-zero the call is **not** queued and
//     the env cookie is removed before raw_call returns. The caller receives
//     a `CallError` describing the system-level failure.
//   * If the call is queued but the callee traps or returns a reject, the
//     registered `on_reject` is invoked with the reject code + message.
//
// Reference:
//   https://internetcomputer.org/docs/current/references/ic-interface-spec/#system-api-call

#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "candid_type.h"
#include "candid_type_all_includes.h"
#include "vec_bytes.h"
#include "wasm_symbol.h"

// ---------------------------------------------------------------------------
// Reject codes mirror the IC system spec (see msg_reject_code).
// ---------------------------------------------------------------------------
enum class CallRejectCode : uint32_t {
  NoError       = 0,
  SysFatal      = 1,
  SysTransient  = 2,
  DestInvalid   = 3,
  CanisterReject= 4,
  CanisterError = 5,
};

struct CallReject {
  CallRejectCode code{CallRejectCode::NoError};
  std::string    msg;
};

// ---------------------------------------------------------------------------
// IC_Call — static façade over the low-level ic0.call_* imports.
// ---------------------------------------------------------------------------
class IC_Call {
public:
  using ReplyHandler  = std::function<void(const VecBytes &reply_bytes)>;
  using RejectHandler = std::function<void(const CallReject &reject)>;

  // Fire an inter-canister call. Returns the system call_perform code:
  //   0          — call queued; one of on_reply / on_reject will run later
  //   non-zero   — call NOT queued; neither callback will run
  //
  // `callee` must be the raw principal blob (use
  // CandidTypePrincipal::get_v_bytes().vec_uint8_t() to obtain it, or pass
  // the CandidTypePrincipal / textual principal overloads below).
  static uint32_t raw_call(const std::vector<uint8_t> &callee,
                           const std::string &method,
                           const std::vector<uint8_t> &candid_args,
                           ReplyHandler  on_reply,
                           RejectHandler on_reject);

  // Convenience: pack a Candid `CandidArgs` then raw_call.
  static uint32_t call(const CandidTypePrincipal &callee,
                       const std::string &method,
                       const CandidArgs &args_out,
                       ReplyHandler  on_reply,
                       RejectHandler on_reject);

  // Convenience: textual principal (e.g. "ryjl3-tyaaa-aaaaa-aaaba-cai")
  static uint32_t call(const std::string &callee_text,
                       const std::string &method,
                       const CandidArgs &args_out,
                       ReplyHandler  on_reply,
                       RejectHandler on_reject);

  // ---- Internal trampoline targets ---------------------------------------
  // These are invoked by canister_reply_callback / canister_reject_callback
  // on the dispatcher entry points. They must NOT be called directly by user
  // code.
  static void dispatch_reply (uint32_t env);
  static void dispatch_reject(uint32_t env);

  // Clear all pending callbacks. Called automatically by pre_upgrade hook in
  // canister_pre_upgrade.cpp.
  static void clear_pending();

  // Diagnostics — number of callbacks still registered.
  static std::size_t pending_count();
};

// ---------------------------------------------------------------------------
// IC_CallBuilder — fluent wrapper for ergonomic call sites:
//
//   IC_CallBuilder(callee_principal, "icrc1_transfer")
//       .with_args(args)
//       .on_reply([](const VecBytes& b){ ... })
//       .on_reject([](const CallReject& r){ ... })
//       .perform();
//
// ---------------------------------------------------------------------------
class IC_CallBuilder {
public:
  IC_CallBuilder(const CandidTypePrincipal &callee, std::string method);
  IC_CallBuilder(const std::string &callee_text, std::string method);

  IC_CallBuilder &with_args  (const CandidArgs &args);
  IC_CallBuilder &on_reply   (IC_Call::ReplyHandler  cb);
  IC_CallBuilder &on_reject  (IC_Call::RejectHandler cb);

  // Returns the call_perform system code. 0 = queued.
  uint32_t perform();

private:
  std::vector<uint8_t>      m_callee_bytes;
  std::string               m_method;
  CandidArgs                m_args;
  bool                      m_have_args{false};
  IC_Call::ReplyHandler     m_on_reply;
  IC_Call::RejectHandler    m_on_reject;
};
