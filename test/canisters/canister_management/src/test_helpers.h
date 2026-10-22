// Helpers shared by the test methods of canister_management

#pragma once

#include <string>

#include "ic_api.h"

// "rej:<code>:<message>" of a rejected call, "err:<error>" of another failure
inline std::string error_text(const IC_CallError &error) {
  if (error.kind == IC_CallError::Kind::CallRejected) {
    return "rej:" + std::to_string(static_cast<uint32_t>(error.reject_code)) +
           ":" + error.reject_message;
  }
  return "err:" + error.to_string();
}

inline std::string nat_text(__uint128_t v) {
  return ICPP_HOOKS::to_string_128(v);
}
