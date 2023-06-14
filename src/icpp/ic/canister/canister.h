// https://internetcomputer.org/docs/current/references/ic-interface-spec#entry-points

#pragma once

#include <variant>

// ------------------------------------------------------------------
// Forward declarations

// The Canister Entry Point base classes
class CanisterBase;

// The Canister Entry Point classes
// https://internetcomputer.org/docs/current/references/ic-interface-spec#entry-points
class CanisterInit;
class CanisterPreUpgrade;
class CanisterPostUpgrade;
class CanisterInspectMessage;
class CanisterHeartbeat;
class CanisterGlobalTimer;
class CanisterUpdate;
class CanisterQuery;
class CanisterReplyCallback;
class CanisterRejectCallback;
class CanisterCleanupCallback;
class CanisterStart;

// The std::variant alias
using CanisterEntry =
    std::variant<CanisterQuery, CanisterUpdate, CanisterInit,
                 CanisterPreUpgrade, CanisterPostUpgrade,
                 CanisterInspectMessage, CanisterHeartbeat, CanisterGlobalTimer,
                 CanisterReplyCallback, CanisterRejectCallback,
                 CanisterCleanupCallback, CanisterStart>;

// https://subscription.packtpub.com/book/programming/9781787120495/8/ch08lvl1sec91/storing-different-types-with-stdvariant
template <typename T> bool is_CanisterEntryPoint(const CanisterEntry &c) {
  return holds_alternative<T>(c);
}

// ------------------------------------------------------------------
// The include files

// The Canister Entry Point base classes
#include "canister_base.h"

// The Canister Entry Point classes
#include "canister_cleanup_callback.h"
#include "canister_global_timer.h"
#include "canister_heartbeat.h"
#include "canister_init.h"
#include "canister_inspect_message.h"
#include "canister_post_upgrade.h"
#include "canister_pre_upgrade.h"
#include "canister_query.h"
#include "canister_reject_callback.h"
#include "canister_reply_callback.h"
#include "canister_start.h"
#include "canister_update.h"