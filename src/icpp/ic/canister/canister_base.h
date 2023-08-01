// The base class for all Canister Entry points

#pragma once

#include <string>

#include "canister.h"

class CanisterBase {
public:
  CanisterBase();
  CanisterBase(std::string calling_function, std::string entry_type);
  ~CanisterBase();

  // Canister Entry Point we're in
  // https://internetcomputer.org/docs/current/references/ic-interface-spec#system-api-imports
  bool is_entry_I() {
    return m_entry_type == "CanisterInit" ||
           m_entry_type == "CanisterPostUpgrade";
  }
  bool is_entry_G() { return m_entry_type == "CanisterPreUpgrade"; }
  bool is_entry_U() { return m_entry_type == "CanisterUpdate"; }
  bool is_entry_Q() { return m_entry_type == "CanisterQuery"; }
  bool is_entry_Ry() { return m_entry_type == "CanisterReplyCallback"; }
  bool is_entry_Rt() { return m_entry_type == "CanisterRejectCallback"; }
  bool is_entry_C() { return m_entry_type == "CanisterCleanupCallback"; }
  bool is_entry_s() { return m_entry_type == "CanisterStart"; }
  bool is_entry_F() { return m_entry_type == "CanisterInspectMessage"; }
  bool is_entry_T() {
    return m_entry_type == "CanisterHeartbeat" ||
           m_entry_type == "CanisterGlobalTimer";
  }
  bool is_entry_Wildcard() { return m_entry_type != "CanisterStart"; }
  std::string get_calling_function() { return m_calling_function; }
  std::string get_entry_type() { return m_entry_type; }

protected:
  // The calling function
  std::string m_calling_function{""};
  // The entry type
  std::string m_entry_type{"CanisterQuery"};
};