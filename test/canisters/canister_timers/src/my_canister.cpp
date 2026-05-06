// A minimal canister demonstrating IC_API timer support.
//
// - start_one_shot()           : schedules a one-shot that increments
//                                g_one_shot_count after a tiny delay.
// - start_recurring(period_ns) : schedules a recurring timer that
//                                increments g_recurring_count every period.
// - stop_timer(id)             : cancels a previously registered timer.
// - get_*                      : query counters / registry size.
// - reset_counts()             : zero counters and cancel all timers.

#include "my_canister.h"

#include <string>

#include "ic_api.h"
#include "ic_timers.h"

uint64_t g_one_shot_count{0};
uint64_t g_recurring_count{0};

// One-shot delay used by start_one_shot. Zero so the deadline lands at "now"
// and the next canister_global_timer tick will fire it.
constexpr uint64_t ONE_SHOT_DELAY_NS = 0;

void canister_init() {
  IC_API ic_api(CanisterInit{std::string(__func__)}, false);
}

void start_one_shot() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  ic_api.from_wire();
  uint64_t id =
      IC_API::set_timer(ONE_SHOT_DELAY_NS, []() { ++g_one_shot_count; });
  ic_api.to_wire(CandidTypeNat64{id});
}

void start_recurring() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  uint64_t period_ns{0};
  ic_api.from_wire(CandidTypeNat64{&period_ns});
  uint64_t id =
      IC_API::set_timer_recurring(period_ns, []() { ++g_recurring_count; });
  ic_api.to_wire(CandidTypeNat64{id});
}

void stop_timer() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  uint64_t id{0};
  ic_api.from_wire(CandidTypeNat64{&id});
  bool ok = IC_API::cancel_timer(id);
  ic_api.to_wire(CandidTypeBool{ok});
}

void reset_counts() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);
  ic_api.from_wire();
  g_one_shot_count = 0;
  g_recurring_count = 0;
  // Cancel every registered timer too — the docstring at the top of this
  // file says reset_counts "zeros counters and cancels all timers", and
  // pytest scenarios depend on a clean registry between scenarios.
  IC_API::cancel_all_timers();
  ic_api.to_wire();
}

void get_one_shot_count() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.from_wire();
  ic_api.to_wire(CandidTypeNat64{g_one_shot_count});
}

void get_recurring_count() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.from_wire();
  ic_api.to_wire(CandidTypeNat64{g_recurring_count});
}

void get_timer_count() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.from_wire();
  ic_api.to_wire(
      CandidTypeNat64{static_cast<uint64_t>(IcTimers::instance().size())});
}
