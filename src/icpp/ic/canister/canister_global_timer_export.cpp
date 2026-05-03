// Definition of the built-in canister_global_timer entry. The declaration
// (with the WASM export attributes) lives in canister_global_timer.h, but
// we include canister.h here instead so the canister_*.h forward-decl
// chain is fully resolved (mirrors how canister_global_timer.cpp does it).

#include "canister.h"

#include "ic_api.h"
#include "ic_timers.h"

void canister_global_timer() {
  IC_API ic_api(CanisterGlobalTimer{std::string(__func__)}, false);
  IcTimers::instance().dispatch_due(IC_API::time());
}
