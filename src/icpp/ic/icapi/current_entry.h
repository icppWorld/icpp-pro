// Process-global "current canister entry point" - internal to icpp.
//
// The IC runs one message at a time, so a canister invocation has exactly
// one entry point and no concurrency (the same reasoning that makes the
// IcTimers singleton safe). The IC_API constructor records its entry here,
// and the canister_global_timer dispatcher records CanisterGlobalTimer
// before dispatching, so IC_API's *static* methods (the timer family,
// time) can enforce the replica's permitted-entry-point rules even though
// they have no IC_API instance.
//
// Guard semantics: current_entry() returns nullptr until the first
// set_current_entry() of the process - callers MUST treat nullptr as
// "skip the guard" (permissive fallback), so code that never constructs
// an IC_API keeps working exactly as before.

#pragma once

#include <cstdint>

#include "canister.h"

void set_current_entry(const CanisterBase &entry);

// Clears the recorded entry. Called by the IC_API destructor, so the entry
// never outlives the IC_API instance that recorded it - an invocation that
// does not construct an IC_API must not inherit a stale entry from the
// previous invocation (a stale guard could falsely trap correct code).
void clear_current_entry();

// nullptr while no IC_API instance is alive; otherwise the entry of the
// current invocation.
CanisterBase *current_entry();

// ---------------------------------------------------------------------------
// Inter-canister calls (ic_async.h).
//
// The IC_API of the current message. An awaited call resumes in a reply or
// reject callback - a new message - and re-attaches the IC_API that lives in
// the coroutine frame, so its entry guards see the callback entry point.
// The pointer is only valid within the message that set it: suspending at a
// co_await clears it. Every constructed IC_API stays registered until its
// destructor runs, so a callback only re-attaches an instance that still
// exists.
class IC_API;
void register_ic_api(IC_API *ic_api); // IC_API constructor: also makes it live
void unregister_ic_api(IC_API *ic_api); // IC_API destructor
void set_live_ic_api(IC_API *ic_api);   // nullptr to clear
IC_API *live_ic_api();
bool is_registered_ic_api(IC_API *ic_api);

// The calls this canister has performed whose callback has not run yet.
// canister_pre_upgrade refuses to upgrade while it is non-zero.
uint32_t calls_in_flight();
void add_calls_in_flight(int32_t delta);

// Traps when calls are in flight and the canister is running (ic_api.cpp).
// The IC_API constructor runs it in canister_pre_upgrade.
void icpp_check_no_calls_in_flight();
