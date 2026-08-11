// my_canister.cpp — test canister for the ic_call inter-canister API.
//
// Exposes:
//   * echo(text) -> text       — trivial echo update (the "callee").
//   * ping_self(text) -> ()    — calls self::echo via IC_Call::call, stores
//                                the reply in g_last_echoed, then replies to
//                                the original caller. Until the inter-call
//                                returns we leave the message open (no
//                                to_wire call from the update entry point);
//                                the reply callback closes it.
//   * get_last_echoed() -> text
//   * get_pending_count() -> nat32

#include "my_canister.h"

#include <string>

#include "ic_api.h"
#include "ic_call.h"
#include "ic0.h"
#include "candid_serialize.h"
#include "candid_deserialize.h"

namespace {
std::string g_last_echoed;
}

void canister_init_() {
  IC_API ic_api(CanisterInit{std::string(__func__)}, false);
  // Note: do NOT call ic_api.to_wire() here. The icpp runtime traps if
  // to_wire is invoked from a CanisterInit context (only U/Q/Ry/Rt allowed).
}

// echo(text) -> text
void echo() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  CandidTypeText payload;
  ic_api.from_wire(payload);

  std::string s = payload.get_v();
  ic_api.to_wire(CandidTypeText{s});
}

// ping_self(text): fire an inter-canister call to self::echo and reply with
// the echoed value once it comes back.
void ping_self() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  CandidTypeText payload;
  ic_api.from_wire(payload);
  std::string s = payload.get_v();

  // The callee is "ic0.canister_self" — i.e. this very canister. IC_Call
  // takes the principal bytes directly, so we copy them from IC_API.
  CandidTypePrincipal self = ic_api.get_canister_self();

  CandidArgs args;
  args.append(CandidTypeText{s});

  const uint32_t code = IC_Call::call(
      self, "echo", args,
      // on_reply
      [](const VecBytes &bytes) {
        // The reply is a Candid-encoded `text`.
        CandidArgs reply_args;
        CandidTypeText t;
        reply_args.append(t);
        CandidDeserialize d(bytes, reply_args);
        g_last_echoed = t.get_v();

        // Close the original message with a Candid-encoded unit reply:
        // "DIDL\0\0" = 0x44 0x49 0x44 0x4c 0x00 0x00.
        static const uint8_t unit_reply[6] = {0x44, 0x49, 0x44, 0x4c, 0x00, 0x00};
        ic0_msg_reply_data_append(reinterpret_cast<uintptr_t>(unit_reply),
                                  sizeof(unit_reply));
        ic0_msg_reply();
      },
      // on_reject
      [](const CallReject &r) {
        std::string msg = "ping_self: callee rejected: " + r.msg;
        ic0_msg_reject(reinterpret_cast<uintptr_t>(msg.data()),
                       static_cast<uint32_t>(msg.size()));
      });

  if (code != 0) {
    std::string msg = "ic0.call_perform failed: code=" + std::to_string(code);
    ic_api.trap(msg);
  }
  // IMPORTANT: do NOT call to_wire() here — the reply will be sent by the
  // on_reply / on_reject callback.
}

void get_last_echoed() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.to_wire(CandidTypeText{g_last_echoed});
}

void get_pending_count() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.to_wire(CandidTypeNat32{static_cast<uint32_t>(
      IC_Call::pending_count())});
}
