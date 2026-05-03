// Multi-timer registry layered on top of the IC's single global timer.
//
// The IC platform exposes one timer per canister via
// ic0.global_timer_set / canister_global_timer. IcTimers extends that into
// a scheduler that supports multiple one-shot and recurring timers, each
// identified by an id that can be passed to cancel().
//
// Inspired by Motoko's prelude/internals.mo. Implemented with std::multimap
// instead of a hand-rolled BST.
//
// Single-threaded: the IC runs one message at a time, so no synchronization
// is needed. Timer state is in-memory only and is not preserved across
// canister upgrades (re-register timers in canister_post_upgrade if needed).

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <unordered_map>

class IcTimers {
public:
  static IcTimers &instance();

  // Schedule a callback to fire once after delay_ns nanoseconds. Returns a
  // non-zero id that can be passed to cancel().
  uint64_t add_one_shot(uint64_t delay_ns, std::function<void()> cb);

  // Schedule a callback to fire repeatedly every period_ns nanoseconds. The
  // first firing is one period after registration. A period of 0 is treated
  // as a one-shot fire-once-at-now timer to match Motoko semantics.
  uint64_t add_recurring(uint64_t period_ns, std::function<void()> cb);

  // Remove a registered timer. Returns true if the id was found.
  bool cancel(uint64_t id);

  // Fire all timers whose deadline is <= now_ns, capped at MAX_PER_TICK.
  // Recurring timers are rescheduled with Motoko-style catch-up.
  // Called from canister_global_timer.
  void dispatch_due(uint64_t now_ns);

  // Number of registered timers (for tests / introspection).
  std::size_t size() const;

private:
  IcTimers() = default;
  IcTimers(const IcTimers &) = delete;
  IcTimers &operator=(const IcTimers &) = delete;

  // Match Motoko's per-tick cap to prevent timer storms from starving other
  // work in the same message budget.
  static constexpr std::size_t MAX_PER_TICK = 10;

  struct Entry {
    uint64_t id{0};
    uint64_t deadline_ns{0};
    uint64_t period_ns{0}; // 0 = one-shot
    std::function<void()> cb;
  };

  using DeadlineIndex = std::multimap<uint64_t, uint64_t>;

  struct Stored {
    Entry entry;
    DeadlineIndex::iterator it;
  };

  uint64_t insert(uint64_t deadline_ns, uint64_t period_ns,
                  std::function<void()> cb);
  void arm_next();

  DeadlineIndex m_by_deadline;
  std::unordered_map<uint64_t, Stored> m_by_id;
  uint64_t m_next_id{1};
  uint64_t m_armed_deadline{0};
};
