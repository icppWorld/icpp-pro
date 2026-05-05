// User-supplied canister_global_timer override.
//
// The icpp-pro side defines canister_global_timer with __attribute__((weak))
// in src/icpp/ic/canister/canister_global_timer_export.cpp. The strong
// definition below takes precedence at link time. To prove the override
// actually wins (and the icpp-pro built-in dispatcher does NOT run), the
// canister also registers a recurring IcTimers timer; if the icpp-pro
// dispatcher had run, that timer's callback would advance
// g_icpp_callback_calls. The test asserts user_dispatcher_calls > 0 AND
// icpp_callback_calls == 0.

#include "my_canister.h"

#include <string>

#include "ic_api.h"
#include "ic_timers.h"

uint64_t g_user_dispatcher_calls{0};
uint64_t g_icpp_callback_calls{0};

namespace {
uint64_t g_user_timer_id{0};
} // namespace

void canister_init() {
  IC_API ic_api(CanisterInit{std::string(__func__)}, false);
}

// The strong override. NO __attribute__((weak)). When linked against
// icpp-pro's weak definition, wasm-ld picks this one.
void canister_global_timer() {
  // The IC's T-entry rules forbid msg_arg/msg_caller here; the IC_API
  // constructor's T-entry guard skips those reads when the entry token is
  // CanisterGlobalTimer.
  IC_API ic_api(CanisterGlobalTimer{std::string(__func__)}, false);
  ++g_user_dispatcher_calls;
  // Deliberately do NOT call IcTimers::instance().dispatch_due(...). The
  // icpp-pro registry stays untouched, which the test verifies by asserting
  // get_icpp_timer_count() stays at 1 after this fires and
  // g_icpp_callback_calls stays at 0.
}

void start_user_timer() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  uint64_t period_ns{0};
  ic_api.from_wire(CandidTypeNat64{&period_ns});
  g_user_timer_id = IC_API::set_timer_recurring(
      period_ns, []() { ++g_icpp_callback_calls; });
  ic_api.to_wire(CandidTypeNat64{g_user_timer_id});
}

void stop_user_timer() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  uint64_t id{0};
  ic_api.from_wire(CandidTypeNat64{&id});
  bool ok = IC_API::cancel_timer(id);
  ic_api.to_wire(CandidTypeBool{ok});
}

void get_user_dispatcher_calls() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.from_wire();
  ic_api.to_wire(CandidTypeNat64{g_user_dispatcher_calls});
}

void get_icpp_callback_calls() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.from_wire();
  ic_api.to_wire(CandidTypeNat64{g_icpp_callback_calls});
}

void get_icpp_timer_count() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.from_wire();
  ic_api.to_wire(
      CandidTypeNat64{static_cast<uint64_t>(IcTimers::instance().size())});
}
