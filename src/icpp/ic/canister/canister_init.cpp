#include "canister.h"

#if defined(__wasm__) || defined(__wasi__)
// In a wasm canister, the icpp toolchain emits the `canister_init` entry
// point as a plain exported function. Unlike a normal WASI program there is
// no `_start` wrapper that calls `__wasm_call_ctors`, which means C++ global
// constructors (e.g. `static std::string g_foo = "bar";`) never run unless
// we explicitly invoke them. Call it once, from the first CanisterInit
// instance, before user code in `canister_init` touches any such globals.
extern "C" void __wasm_call_ctors();

namespace {
struct CtorRunner {
  CtorRunner() {
    // Guard: only run once even if a canister somehow constructs CanisterInit
    // multiple times (e.g. in a custom upgrade path).
    static bool s_done = false;
    if (!s_done) {
      s_done = true;
      __wasm_call_ctors();
    }
  }
};
} // namespace
#endif

CanisterInit::CanisterInit() : CanisterBase() {
#if defined(__wasm__) || defined(__wasi__)
  static CtorRunner s_runner;
#endif
}

CanisterInit::CanisterInit(std::string calling_function)
    : CanisterBase(calling_function, std::string(__func__)) {
#if defined(__wasm__) || defined(__wasi__)
  static CtorRunner s_runner;
#endif
}

CanisterInit::~CanisterInit() {}