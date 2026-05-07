// Definition of the built-in canister_global_timer entry. The declaration
// (with the WASM export attributes) lives in canister_global_timer.h, but
// we include canister.h here instead so the canister_*.h forward-decl
// chain is fully resolved (mirrors how canister_global_timer.cpp does it).
//
// __attribute__((weak)) on the definition makes this symbol a weak
// definition so a user-supplied strong canister_global_timer (declared via
// the same WASM_SYMBOL_EXPORTED macro and defined without weak in the
// user's translation unit) takes precedence at link time. wasm-ld picks
// the strong definition; our archive member is then not pulled in.

#include "canister.h"

#include "ic_api.h"
#include "ic_timers.h"

__attribute__((weak)) void canister_global_timer() {
  IC_API ic_api(CanisterGlobalTimer{std::string(__func__)}, false);
  IcTimers::instance().dispatch_due(IC_API::time());
}
