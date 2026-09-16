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
