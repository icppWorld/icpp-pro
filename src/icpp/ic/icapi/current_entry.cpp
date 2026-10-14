#include "current_entry.h"

#include "canister_base.h"

#include <unordered_set>

#if defined(__wasm__)
// wasi_sdk_traps/__wasilibc_initialize_environ.c: getenv() reads the canister's
// environment variables while an entry point other than canister_start runs.
extern "C" void icpp_environ_set_available(int available);
#endif

namespace {
CanisterBase g_current_entry;
bool g_current_entry_set{false};
IC_API *g_live_api{nullptr};
uint32_t g_calls_in_flight{0};

std::unordered_set<IC_API *> &registered_apis() {
  static std::unordered_set<IC_API *> apis;
  return apis;
}
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

void register_api(IC_API *api) {
  registered_apis().insert(api);
  g_live_api = api;
}

void unregister_api(IC_API *api) {
  registered_apis().erase(api);
  if (g_live_api == api) g_live_api = nullptr;
}

void set_live_api(IC_API *api) { g_live_api = api; }

IC_API *live_api() { return g_live_api; }

bool is_registered_api(IC_API *api) { return registered_apis().count(api) > 0; }

uint32_t calls_in_flight() { return g_calls_in_flight; }

void add_calls_in_flight(int32_t delta) {
  if (delta < 0 && g_calls_in_flight < (uint32_t)(-delta)) {
    g_calls_in_flight = 0;
  } else {
    g_calls_in_flight += delta;
  }
}
