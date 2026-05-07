#include "ic_timers.h"

#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

#include "ic0.h"

namespace {
// Saturating uint64_t addition. Used everywhere we add a delay/period to a
// nanosecond timestamp so a near-UINT64_MAX argument can't wrap the deadline
// into the past and fire immediately. Saturating to UINT64_MAX is acceptable
// because the IC's global timer accepts any uint64_t and "the very far
// future" is the right behavior for absurd delays.
inline uint64_t saturating_add_u64(uint64_t a, uint64_t b) {
  uint64_t max = std::numeric_limits<uint64_t>::max();
  return (b > max - a) ? max : a + b;
}
} // namespace

IcTimers &IcTimers::instance() {
  static IcTimers s;
  return s;
}

uint64_t IcTimers::insert(uint64_t deadline_ns, uint64_t period_ns,
                          std::function<void()> cb) {
  uint64_t id = m_next_id++;
  auto it = m_by_deadline.insert({deadline_ns, id});
  Stored s;
  s.entry.id = id;
  s.entry.deadline_ns = deadline_ns;
  s.entry.period_ns = period_ns;
  s.entry.cb = std::move(cb);
  s.it = it;
  m_by_id.emplace(id, std::move(s));
  arm_next();
  return id;
}

uint64_t IcTimers::add_one_shot(uint64_t delay_ns, std::function<void()> cb) {
  uint64_t deadline = saturating_add_u64(ic0_time(), delay_ns);
  return insert(deadline, 0, std::move(cb));
}

uint64_t IcTimers::add_recurring(uint64_t period_ns, std::function<void()> cb) {
  uint64_t deadline = saturating_add_u64(ic0_time(), period_ns);
  return insert(deadline, period_ns, std::move(cb));
}

bool IcTimers::cancel(uint64_t id) {
  auto sit = m_by_id.find(id);
  if (sit == m_by_id.end()) return false;
  m_by_deadline.erase(sit->second.it);
  m_by_id.erase(sit);
  arm_next();
  return true;
}

std::size_t IcTimers::size() const { return m_by_id.size(); }

void IcTimers::clear() {
  m_by_deadline.clear();
  m_by_id.clear();
  m_armed_deadline = 0;
  // Unconditionally disarm — do NOT defer to arm_next()'s cache-equality
  // skip here. After raw / custom interactions or dispatch edges the cache
  // may not reflect IC state; correctness beats saving one syscall.
  ic0_global_timer_set(0);
}

void IcTimers::arm_next() {
  uint64_t target = m_by_deadline.empty() ? 0 : m_by_deadline.begin()->first;
  if (target != m_armed_deadline) {
    ic0_global_timer_set(target);
    m_armed_deadline = target;
  }
}

void IcTimers::dispatch_due(uint64_t now_ns) {
  // The IC fired our one-shot global timer to invoke this entry, so whatever
  // we had armed is consumed at the IC level. Invalidate the cache so
  // arm_next() will re-arm unconditionally — otherwise, when the remaining
  // earliest deadline equals the previously-armed value (e.g. >MAX_PER_TICK
  // timers all sharing a deadline), arm_next would skip the syscall and the
  // remaining timers would be permanently stranded.
  m_armed_deadline = 0;

  // Gather phase: collect IDs of due timers (up to MAX_PER_TICK), then
  // mutate the registry. We deliberately store *ids*, not callback copies,
  // so a callback that calls cancel(other_id) or cancel_all_timers() during
  // the execute phase below actually prevents the targeted timer from
  // firing — the lookup in execute consults the live m_by_id and skips
  // entries that have been removed mid-batch. The earlier copy-cb-and-run
  // design fired cancelled timers anyway because the cb had already been
  // copied; that was observably wrong cancel semantics.
  std::vector<uint64_t> to_run;
  to_run.reserve(MAX_PER_TICK);

  while (to_run.size() < MAX_PER_TICK) {
    auto it = m_by_deadline.begin();
    if (it == m_by_deadline.end() || it->first > now_ns) break;

    uint64_t id = it->second;
    auto sit = m_by_id.find(id);
    if (sit == m_by_id.end()) {
      // Defensive: deadline index out of sync — drop the orphan and continue.
      m_by_deadline.erase(it);
      continue;
    }

    to_run.push_back(id);

    if (sit->second.entry.period_ns > 0) {
      uint64_t pivot = sit->second.entry.deadline_ns;
      uint64_t period = sit->second.entry.period_ns;
      // Motoko catch-up: skip missed periods so we don't fire-storm after
      // a slow tick. Always lands strictly past now_ns. Saturating add
      // because the user's chosen period plus pivot can overflow on
      // adversarial inputs; we'd rather "never fires again" than wrap to
      // the past and fire immediately in a tight loop.
      uint64_t skipped = (now_ns >= pivot) ? (now_ns - pivot) / period : 0;
      uint64_t advance = saturating_add_u64(skipped, 1);
      // next = pivot + period * advance, computed carefully:
      uint64_t increment =
          (period > std::numeric_limits<uint64_t>::max() / advance)
              ? std::numeric_limits<uint64_t>::max()
              : period * advance;
      uint64_t next = saturating_add_u64(pivot, increment);
      m_by_deadline.erase(it);
      sit->second.entry.deadline_ns = next;
      sit->second.it = m_by_deadline.insert({next, id});
    } else {
      // One-shot: remove from the deadline index now (so the next gather
      // iteration sees the next entry), but keep it in m_by_id so that
      //   (a) the execute phase can still find the cb at run time, and
      //   (b) cancel(id) called from another callback in this batch can
      //       remove it and prevent it from firing.
      m_by_deadline.erase(it);
    }
  }

  // Execute phase: look up each id at run time. If a previous callback
  // cancelled this id (or cleared the whole registry via cancel_all_timers),
  // m_by_id.find(id) returns end and we skip — that's the cancel-mid-batch
  // semantics the gather phase preserves above.
  //
  // For one-shots we also erase the entry from m_by_id BEFORE invoking the
  // cb, so a cb that calls cancel(its-own-id) or set_timer() observes a
  // consistent registry state ("I'm no longer registered, anything I add
  // now is fresh").
  //
  // Note: WASM is built with -fno-exceptions; if a callback traps the
  // entire message rolls back, including the registry mutations above.
  for (uint64_t id : to_run) {
    auto sit = m_by_id.find(id);
    if (sit == m_by_id.end()) continue;
    auto cb = sit->second.entry.cb; // copy, in case cb mutates m_by_id
    if (sit->second.entry.period_ns == 0) {
      m_by_id.erase(sit);
    }
    cb();
  }

  arm_next();
}
