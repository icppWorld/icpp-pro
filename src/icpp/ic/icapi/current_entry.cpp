#include "current_entry.h"

#include "canister_base.h"

namespace {
CanisterBase g_current_entry;
bool g_current_entry_set{false};
} // namespace

void set_current_entry(const CanisterBase &entry) {
  g_current_entry = entry;
  g_current_entry_set = true;
}

CanisterBase *current_entry() {
  if (!g_current_entry_set) {
    return nullptr;
  }
  return &g_current_entry;
}
