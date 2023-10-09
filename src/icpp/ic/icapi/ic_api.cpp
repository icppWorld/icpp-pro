// API functions to be called by a C++ application to interact with ic0
// functions
//
// References:
//  -
//  https://smartcontracts.org/docs/interface-spec/index.html#system-api-imports
//  - https://github.com/dfinity/candid/blob/master/spec/Candid.md#binary-format

#include "ic_api.h"
#include "candid_deserialize.h"
#include "candid_serialize.h"
#include "candid_serialize_type_table_registry.h"
#include "candid_type.h"
#include "candid_type_all_includes.h"

#include <cassert>
#include <cstddef>
#include <cstring>
#include <limits.h>
#include <string>

#include "ic0.h"

IC_API::IC_API() : IC_API(CanisterQuery("-unknown-"), false) {}
IC_API::IC_API(const bool &debug_print)
    : IC_API(CanisterQuery("-unknown-"), debug_print) {}

IC_API::IC_API(const CanisterBase &canister_entry, const bool &dbug)
    : m_canister_entry(canister_entry), m_debug_print(dbug) {
  // Always start with a clean type table registry in the registry singleton
  CandidSerializeTypeTableRegistry::get_instance().clear();
  // Fill 'm_B_in' with the bytes of msg_arg_data
  std::vector<uint8_t> bytes(ic0_msg_arg_data_size());
  ic0_msg_arg_data_copy(reinterpret_cast<uintptr_t>(bytes.data()), 0,
                        bytes.size());
  m_B_in.store(bytes.data(), bytes.size());

  // Get the principal of caller
  std::vector<uint8_t> bytes_caller(ic0_msg_caller_size());
  ic0_msg_caller_copy(reinterpret_cast<uintptr_t>(bytes_caller.data()), 0,
                      bytes_caller.size());
  m_caller = CandidTypePrincipal(bytes_caller);

  if (m_debug_print) {
    debug_print("\n--");
    debug_print("IC_API caller's principal:" + m_caller.get_text());
    debug_print("IC_API received these bytes over the wire:");
    m_B_in.debug_print();
  }
}

IC_API::~IC_API() {}

void IC_API::debug_print(const char *message) {

  // https:
  // //wiki.sei.cmu.edu/confluence/display/c/INT36-C.+Converting+a+pointer+to+integer+or+integer+to+pointer
  //
  // Use `(uintptr_t)(void *)message` instead of `(uintptr_t)(void *)message`:
  // -> Either way works in IC canister, because target=wasm32 uses 32 bit
  // pointers.
  // -> The first approach is portable, and also works for:
  //     (-) native executable with 64 bit pointers
  //     (-) ports to wasm64 with 64 bit pointers

  ic0_debug_print((uintptr_t)(void *)message, (uint32_t)strlen(message));
}

CandidTypePrincipal IC_API::get_caller() { return m_caller; }

void IC_API::debug_print(const std::string &s) {
  IC_API::debug_print(s.c_str());
}

void IC_API::trap(const char *message) {
  ic0_trap((uintptr_t)(void *)message, (uint32_t)strlen(message));
}

void IC_API::trap(const std::string &s) { IC_API::trap(s.c_str()); }

uint64_t IC_API::time() { return ic0_time(); }

// DeSerialize the byte stream received over the wire
void IC_API::from_wire(CandidArgs A) {
  if (m_called_from_wire) {
    trap("ERROR: The canister is calling ic_api.from_wire() more than once.");
  }
  m_called_from_wire = true;

  CandidDeserialize(m_B_in, A);

  // Reset the type table registry singleton
  CandidSerializeTypeTableRegistry::get_instance().clear();
}
// DeSerialize the byte stream received over the wire
void IC_API::from_wire(CandidType c) {
  CandidArgs A;
  A.append(c);
  from_wire(A);
}
// DeSerialize the byte stream received over the wire, when no arguments are expected
void IC_API::from_wire() {
  CandidArgs A;
  from_wire(A);
}

std::optional<__uint128_t> IC_API::string_to_uint128_t(const std::string &str) {
  __uint128_t result = 0;
  for (char c : str) {
    if (c < '0' || c > '9')
      return std::
          nullopt; // return an empty optional if the string contains a non-digit character

    __uint128_t prev_result = result;
    result = result * 10 + (c - '0');

    // If the new result is smaller than the previous result, that means an overflow occurred,
    // so return an empty optional
    if (result < prev_result) return std::nullopt;
  }

  return result;
}

std::optional<__int128_t> IC_API::string_to_int128_t(const std::string &str) {
  bool isNegative = false;
  std::string str_copy = str;

  if (str_copy[0] == '-') {
    isNegative = true;
    str_copy.erase(0, 1); // remove the '-' sign
  }

  // Call the string_to_128 function for __uint128_t
  std::optional<__uint128_t> res = string_to_uint128_t(str_copy);

  // If conversion was unsuccessful, return an empty optional
  if (!res) return std::nullopt;

  if (isNegative) {
    return -static_cast<__int128_t>(
        *res); // convert the result to negative if the original string started with '-'
  } else {
    return static_cast<__int128_t>(*res);
  }
}

// to_string for __uint128_t, which is not provided by std::to_string
std::string IC_API::to_string_128(__uint128_t v) {
  // https://stackoverflow.com/a/55970931/5480536
  std::string str;
  do {
    int digit = v % 10;
    str = std::to_string(digit) + str;
    v = (v - digit) / 10;
  } while (v != 0);
  return str;
}

// to_string for __int128_t, which is not provided by std::to_string
std::string IC_API::to_string_128(__int128_t v) {
  std::string str;
  bool isNegative = false;
  if (v < 0) {
    isNegative = true;
    str = to_string_128(__uint128_t(-v));
  } else {
    str = to_string_128(__uint128_t(v));
  }
  if (isNegative) {
    str = "-" + str;
  }
  return str;
}

// Serialize to byte stream & send out over the wire, when no arguments are sent
void IC_API::to_wire() {
  CandidArgs args_out;
  to_wire(args_out);
}

// Serialize to byte stream & send out over the wire
void IC_API::to_wire(const CandidType &arg_out) {
  CandidArgs args_out;
  args_out.append(arg_out);
  to_wire(args_out);
}

void IC_API::to_wire(const CandidArgs &args_out) {
  if (m_called_to_wire) {
    trap("ERROR: The canister is calling ic_api.to_wire() more than once.");
  }
  if (!(m_canister_entry.is_entry_U() || m_canister_entry.is_entry_Q() ||
        m_canister_entry.is_entry_Ry() || m_canister_entry.is_entry_Rt())) {
    trap(
        "ERROR: The canister function '" +
        m_canister_entry.get_calling_function() +
        "' is calling ic_api.to_wire(), but it is of type '" +
        m_canister_entry.get_entry_type() +
        "'.\nYou can only call ic_api.to_wire() for CanisterQuery, CanisterUpdate, CanisterReplyCallback & CanisterRejectCallback functions.\nPlease update the IC_API instantiation.");
  }
  m_called_to_wire = true;

  m_B_out = CandidSerialize(args_out).get_B();

  // Optionally, debug print the final result
  if (m_debug_print) {
    debug_print("\n--");
    debug_print("IC_API 'to_wire' is sending these bytes back:");
    m_B_out.debug_print();
  }

  // https://internetcomputer.org/docs/current/references/ic-interface-spec#system-api-imports
  // Only can call msg_reply if entry is `U Q Ry Rt`
  if (m_canister_entry.is_entry_U() || m_canister_entry.is_entry_Q() ||
      m_canister_entry.is_entry_Ry() || m_canister_entry.is_entry_Rt()) {
    // If the method did not yet call to_wire, do it automatic without content
    if (!m_called_to_wire) {
      to_wire();
    }
    // Send it out over the wire
    msg_reply();
  }

  // Reset the type table registry singleton
  CandidSerializeTypeTableRegistry::get_instance().clear();
}

// Appends the content of m_B_out, replies over the wire, re-sets the didl vectors
void IC_API::msg_reply() {

  m_B_out.trap_if_vec_does_not_start_with_DIDL();

  ic0_msg_reply_data_append((uintptr_t)(void *)m_B_out.vec().data(),
                            (uint32_t)m_B_out.size());
  // clear the didl vectors
  m_B_in.clear();
  m_B_out.clear();

  // send it
  ic0_msg_reply();
}