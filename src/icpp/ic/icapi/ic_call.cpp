// Inter-canister call helper — see ic_call.h for design notes.
//
// The IC's call_new takes two function-pointer offsets (reply_fun /
// reject_fun) into the WASM table plus an `env` u32 cookie that is passed
// back to the callback unmodified. We expose two fixed exported symbols
// `__icpp_call_reply` and `__icpp_call_reject` whose addresses we resolve
// via inline asm-name labels, and dispatch by env cookie into a C++ map of
// pending callbacks.

#include "ic_call.h"

#include <cstdint>
#include <cstring>
#include <unordered_map>

#include "candid_serialize.h"
#include "ic0.h"
#include "ic_api.h"

namespace {

struct Pending {
  IC_Call::ReplyHandler  on_reply;
  IC_Call::RejectHandler on_reject;
};

// env cookie ? handler pair. Cleared by post_upgrade.
std::unordered_map<uint32_t, Pending> g_pending;
uint32_t                              g_next_env = 1;

CallRejectCode code_from_u32(uint32_t c) {
  if (c >= 1 && c <= 5) return static_cast<CallRejectCode>(c);
  return CallRejectCode::NoError;
}

// Pack a CandidArgs into a serialised byte vector.
std::vector<uint8_t> serialize_args(const CandidArgs &args) {
  VecBytes B = CandidSerialize(args).get_B();
  // VecBytes::vec_uint8_t() returns the underlying std::vector<uint8_t>
  // companion buffer (vec() returns the std::byte form).
  return B.vec_uint8_t();
}

// Decode a textual principal to its raw bytes via CandidTypePrincipal.
std::vector<uint8_t> principal_text_to_bytes(const std::string &text) {
  CandidTypePrincipal p(text);
  return p.get_v_bytes().vec_uint8_t();
}

// Extract raw bytes from a CandidTypePrincipal.
std::vector<uint8_t> principal_to_bytes(const CandidTypePrincipal &p) {
  return p.get_v_bytes().vec_uint8_t();
}

} // namespace

// ---------------------------------------------------------------------------
// Exported reply / reject trampolines
//
// These are the targets passed to ic0.call_new. They look up the per-call
// callback in g_pending using the env cookie, copy the reply payload (if
// any) into a VecBytes, invoke the C++ handler, and erase the entry.
// ---------------------------------------------------------------------------

extern "C" void __icpp_call_reply_trampoline(uint32_t env)
    WASM_SYMBOL_EXPORTED("__icpp_call_reply_trampoline");

extern "C" void __icpp_call_reject_trampoline(uint32_t env)
    WASM_SYMBOL_EXPORTED("__icpp_call_reject_trampoline");

extern "C" void __icpp_call_reply_trampoline(uint32_t env) {
  IC_Call::dispatch_reply(env);
}

extern "C" void __icpp_call_reject_trampoline(uint32_t env) {
  IC_Call::dispatch_reject(env);
}

// ---------------------------------------------------------------------------
// IC_Call
// ---------------------------------------------------------------------------

uint32_t IC_Call::raw_call(const std::vector<uint8_t> &callee,
                           const std::string          &method,
                           const std::vector<uint8_t> &candid_args,
                           ReplyHandler                on_reply,
                           RejectHandler               on_reject) {
  // Reserve an env cookie before we touch ic0; if call_perform fails we
  // remove it again.
  const uint32_t env = g_next_env++;
  g_pending.emplace(env, Pending{std::move(on_reply), std::move(on_reject)});

  // The trampoline function "pointers" we hand to ic0.call_new are the
  // addresses of the exported callbacks in the WASM function table. Cast
  // through uintptr_t to placate the system API which wants a u32 offset.
  const uintptr_t reply_fun =
      reinterpret_cast<uintptr_t>(&__icpp_call_reply_trampoline);
  const uintptr_t reject_fun =
      reinterpret_cast<uintptr_t>(&__icpp_call_reject_trampoline);

  ic0_call_new(
      reinterpret_cast<uintptr_t>(callee.data()),
      static_cast<uint32_t>(callee.size()),
      reinterpret_cast<uintptr_t>(method.data()),
      static_cast<uint32_t>(method.size()),
      static_cast<uint32_t>(reply_fun),
      env,
      static_cast<uint32_t>(reject_fun),
      env);

  if (!candid_args.empty()) {
    ic0_call_data_append(reinterpret_cast<uintptr_t>(candid_args.data()),
                         static_cast<uint32_t>(candid_args.size()));
  }

  const uint32_t code = ic0_call_perform();
  if (code != 0) {
    // System refused to queue the call — drop the callback so we don't leak
    // it forever.
    g_pending.erase(env);
  }
  return code;
}

uint32_t IC_Call::call(const CandidTypePrincipal &callee,
                       const std::string &method,
                       const CandidArgs &args_out,
                       ReplyHandler  on_reply,
                       RejectHandler on_reject) {
  return raw_call(principal_to_bytes(callee), method, serialize_args(args_out),
                  std::move(on_reply), std::move(on_reject));
}

uint32_t IC_Call::call(const std::string &callee_text,
                       const std::string &method,
                       const CandidArgs &args_out,
                       ReplyHandler  on_reply,
                       RejectHandler on_reject) {
  return raw_call(principal_text_to_bytes(callee_text), method,
                  serialize_args(args_out),
                  std::move(on_reply), std::move(on_reject));
}

void IC_Call::dispatch_reply(uint32_t env) {
  auto it = g_pending.find(env);
  if (it == g_pending.end()) {
    IC_API::debug_print("ic_call: stale reply env, no handler");
    return;
  }
  auto handler = std::move(it->second.on_reply);
  g_pending.erase(it);

  // Copy reply bytes (msg_arg_data_*) into a VecBytes.
  VecBytes reply_bytes;
  const uint32_t sz = ic0_msg_arg_data_size();
  if (sz > 0) {
    std::vector<uint8_t> buf(sz);
    ic0_msg_arg_data_copy(reinterpret_cast<uintptr_t>(buf.data()), 0, sz);
    reply_bytes.store(buf.data(), buf.size());
  }

  if (handler) handler(reply_bytes);
}

void IC_Call::dispatch_reject(uint32_t env) {
  auto it = g_pending.find(env);
  if (it == g_pending.end()) {
    IC_API::debug_print("ic_call: stale reject env, no handler");
    return;
  }
  auto handler = std::move(it->second.on_reject);
  g_pending.erase(it);

  CallReject r;
  r.code = code_from_u32(ic0_msg_reject_code());
  const uint32_t sz = ic0_msg_reject_msg_size();
  if (sz > 0) {
    std::vector<char> buf(sz);
    ic0_msg_reject_msg_copy(reinterpret_cast<uintptr_t>(buf.data()), 0, sz);
    r.msg.assign(buf.data(), buf.size());
  }
  if (handler) handler(r);
}

void IC_Call::clear_pending() {
  g_pending.clear();
  g_next_env = 1;
}

std::size_t IC_Call::pending_count() {
  return g_pending.size();
}

// ---------------------------------------------------------------------------
// IC_CallBuilder
// ---------------------------------------------------------------------------

IC_CallBuilder::IC_CallBuilder(const CandidTypePrincipal &callee,
                               std::string method)
    : m_callee_bytes(principal_to_bytes(callee)), m_method(std::move(method)) {}

IC_CallBuilder::IC_CallBuilder(const std::string &callee_text,
                               std::string method)
    : m_callee_bytes(principal_text_to_bytes(callee_text)),
      m_method(std::move(method)) {}

IC_CallBuilder &IC_CallBuilder::with_args(const CandidArgs &args) {
  m_args      = args;
  m_have_args = true;
  return *this;
}

IC_CallBuilder &IC_CallBuilder::on_reply(IC_Call::ReplyHandler cb) {
  m_on_reply = std::move(cb);
  return *this;
}

IC_CallBuilder &IC_CallBuilder::on_reject(IC_Call::RejectHandler cb) {
  m_on_reject = std::move(cb);
  return *this;
}

uint32_t IC_CallBuilder::perform() {
  std::vector<uint8_t> serialised =
      m_have_args ? serialize_args(m_args) : std::vector<uint8_t>{};
  return IC_Call::raw_call(m_callee_bytes, m_method, serialised,
                           std::move(m_on_reply), std::move(m_on_reject));
}
