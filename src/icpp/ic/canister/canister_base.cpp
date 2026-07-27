// The base class for all Canister Entry points

#include "canister.h"

#include "candid_type.h"
#include "candid_type_all_includes.h"
#include "ic_api.h"

#if defined(__wasm__) || defined(__wasi__)
extern "C" void __wasm_call_ctors();
#endif

// Run C++ global constructors exactly once per wasm instance.
//
// The icpp toolchain emits canister entry points as plain exported
// functions; there is no `_start` wrapper, so `__wasm_call_ctors()` never
// runs unless an entry point invokes it. Crucially, a wasm instance is
// created fresh on *every* code install — including `--mode upgrade`,
// where `canister_init` does NOT run. If only canister_init triggers the
// ctors, an upgraded instance runs forever with zero-initialized globals:
// a global `std::string` traps on use, and a global `std::unordered_map`
// has max_load_factor == 0.0, which makes libc++ double its bucket array
// on every insert (this bricked a canister at 3.37 GiB heap after ~28
// inter-canister calls).
//
// CanisterBase's constructors call this, so the first IC_API construction
// of ANY entry type (init, post_upgrade, update, query, callbacks) runs
// the ctors. Canisters whose canister_post_upgrade does not construct an
// IC_API should call this helper directly at the top of post_upgrade,
// BEFORE touching any global with a dynamic initializer — note that any
// state already stored in such globals is overwritten when the ctors run.
extern "C" void icpp_run_global_ctors_once() {
#if defined(__wasm__) || defined(__wasi__)
  static bool s_done = false; // zero-initialized, safe before ctors run
  if (!s_done) {
    s_done = true;
    __wasm_call_ctors();
  }
#endif
}

CanisterBase::CanisterBase() { icpp_run_global_ctors_once(); }

CanisterBase::CanisterBase(std::string calling_function,
                           std::string entry_type) {
  icpp_run_global_ctors_once();
  m_calling_function = calling_function;
  m_entry_type = entry_type;
}

CanisterBase::~CanisterBase() {}
