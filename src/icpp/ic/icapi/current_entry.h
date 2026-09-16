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

// nullptr until the first set_current_entry(); afterwards, the entry of
// the current (or most recent) invocation.
CanisterBase *current_entry();
