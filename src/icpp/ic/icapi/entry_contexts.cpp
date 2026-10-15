#include "entry_contexts.h"

#include "ic0.h"

namespace entry_contexts {

bool all_except_start(CanisterBase &entry) { return !entry.is_entry_s(); }

bool root_key(CanisterBase &entry) {
  return entry.is_entry_I() || entry.is_entry_G() || entry.is_entry_U() ||
         entry.is_entry_Ry() || entry.is_entry_Rt() || entry.is_entry_C() ||
         entry.is_entry_T() ||
         (entry.is_entry_Q() && ic0_in_replicated_execution() == 1);
}

bool caller_info(CanisterBase &entry) {
  return entry.is_entry_U() || entry.is_entry_Q() || entry.is_entry_Ry() ||
         entry.is_entry_Rt() || entry.is_entry_C() || entry.is_entry_F();
}

bool call(CanisterBase &entry) {
  return entry.is_entry_U() || entry.is_entry_Ry() || entry.is_entry_Rt() ||
         entry.is_entry_T();
}

bool msg_deadline(CanisterBase &entry) {
  return entry.is_entry_U() || entry.is_entry_Q() || entry.is_entry_Ry() ||
         entry.is_entry_Rt();
}

} // namespace entry_contexts
