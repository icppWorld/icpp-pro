#include "current_entry.h"

#include "canister_base.h"

#if defined(__wasm__)
// wasi_sdk_traps/__wasilibc_initialize_environ.c: getenv() reads the canister's
// environment variables while an entry point other than canister_start runs.
extern "C" void icpp_environ_set_available(int available);
#endif

namespace {
CanisterBase g_current_entry;
bool g_current_entry_set{false};
} // namespace

void set_current_entry(const CanisterBase &entry) {
  g_current_entry = entry;
  g_current_entry_set = true;
#if defined(__wasm__)
  icpp_environ_set_available(!g_current_entry.is_entry_s());
#endif
}

void clear_current_entry() {
  g_current_entry_set = false;
#if defined(__wasm__)
  icpp_environ_set_available(0);
#endif
}

CanisterBase *current_entry() {
  if (!g_current_entry_set) {
    return nullptr;
  }
  return &g_current_entry;
}
