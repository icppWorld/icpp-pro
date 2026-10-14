// The entry points in which the IC permits a system API - the availability
// table of the IC interface spec - internal to icpp.
//
// Shared by the IC_API guards and the Mock IC, so the two cannot disagree:
// IC_API traps with a clear message before calling ic0, and the Mock IC
// refuses a direct ic0 call the same way the replica does.

#pragma once

#include "canister.h"

namespace entry_contexts {

// The spec's "*": every entry point except canister_start.
bool all_except_start(CanisterBase &entry);

// init, pre/post_upgrade, update, reply & reject callbacks, cleanup,
// heartbeat & timer, and a query in replicated execution. A query call
// (non-replicated) and inspect_message are refused.
bool root_key(CanisterBase &entry);

// update, query, reply & reject callbacks, cleanup and inspect_message.
bool caller_info(CanisterBase &entry);

// update, reply & reject callbacks, heartbeat & timer: where call_new and the
// rest of the call API are available. (Composite queries are not supported.)
bool call(CanisterBase &entry);

// update, query, reply & reject callbacks.
bool msg_deadline(CanisterBase &entry);

} // namespace entry_contexts
