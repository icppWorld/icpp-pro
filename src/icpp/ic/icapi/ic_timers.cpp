#include "ic_timers.h"

#include <utility>
#include <vector>

#include "ic0.h"

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
  uint64_t deadline = ic0_time() + delay_ns;
  return insert(deadline, 0, std::move(cb));
}

uint64_t IcTimers::add_recurring(uint64_t period_ns, std::function<void()> cb) {
  uint64_t deadline = ic0_time() + period_ns;
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

  std::vector<std::function<void()>> to_run;
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

    to_run.push_back(sit->second.entry.cb);

    if (sit->second.entry.period_ns > 0) {
      uint64_t pivot = sit->second.entry.deadline_ns;
      uint64_t period = sit->second.entry.period_ns;
      // Motoko catch-up: skip missed periods so we don't fire-storm after
      // a slow tick. Always lands strictly past now_ns.
      uint64_t next = (now_ns >= pivot)
                          ? pivot + period * (1 + (now_ns - pivot) / period)
                          : pivot + period;
      m_by_deadline.erase(it);
      sit->second.entry.deadline_ns = next;
      sit->second.it = m_by_deadline.insert({next, id});
    } else {
      m_by_deadline.erase(it);
      m_by_id.erase(sit);
    }
  }

  // Run callbacks before re-arming, so any timers they register or cancel
  // are reflected in the value we hand to ic0.global_timer_set.
  // Note: WASM is built with -fno-exceptions; if a callback traps the
  // entire message rolls back, including the registry mutations above.
  for (auto &cb : to_run)
    cb();

  arm_next();
}
