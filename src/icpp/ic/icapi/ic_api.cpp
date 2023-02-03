// API functions to be called by a C++ application to interact with ic0
// functions
//
// References:
//  -
//  https://smartcontracts.org/docs/interface-spec/index.html#system-api-imports
//  - https://github.com/dfinity/candid/blob/master/spec/Candid.md#binary-format

#include "ic_api.h"

#include <cassert>
#include <cstddef>
#include <cstring>
#include <limits.h>

#include <string>
// #include <sstream>
// #include <iomanip>

#include "ic0.h"

IC_API::IC_API() {}

IC_API::IC_API(const bool &dbug) : m_debug_print(dbug) {
  // Fill 'm_B_in' with the bytes of msg_arg_data
  uint32_t num_bytes = ic0_msg_arg_data_size();
  uint8_t *bytes = (uint8_t *)(malloc(num_bytes));
  ic0_msg_arg_data_copy((uintptr_t)(void *)(bytes), 0, (uint32_t)(num_bytes));
  m_B_in.store(bytes, num_bytes);
  free(bytes);

  if (m_debug_print) {
    debug_print("\n--");
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

void IC_API::debug_print(const std::string &s) {
  IC_API::debug_print(s.c_str());
}

void IC_API::trap(const char *message) {
  ic0_trap((uintptr_t)(void *)message, (uint32_t)strlen(message));
}

void IC_API::trap(const std::string &s) { IC_API::trap(s.c_str()); }

uint64_t IC_API::time() { return ic0_time(); }

// DeSerialize the byte stream received over the wire
void IC_API::from_wire(std::vector<CandidType> A) {
  CandidDeserialize(m_B_in, A);
}
// DeSerialize the byte stream received over the wire
void IC_API::from_wire(CandidType c) {
  std::vector<CandidType> A;
  A.push_back(c);
  from_wire(A);
}

// Serialize to byte stream & send out over the wire
void IC_API::to_wire(const CandidType &arg_out) {
  std::vector<CandidType> args_out;
  args_out.push_back(arg_out);
  to_wire(args_out);
}

void IC_API::to_wire(const std::vector<CandidType> &args_out) {

  m_B_out = CandidSerialize(args_out).get_B();

  // Optionally, debug print the final result
  if (m_debug_print) {
    debug_print("\n--");
    debug_print("IC_API 'to_wire' is sending these bytes back:");
    m_B_out.debug_print();
  }

  // Send it out over the wire
  msg_reply();
}

// Appends the content of m_B_out, replies over the wire, re-sets the didl vectors
void IC_API::msg_reply() {

  m_B_out.trap_if_vec_does_not_start_with_DIDL();

  ic0_msg_reply_data_append((uintptr_t)(void *)m_B_out.vec().data(),
                            (uint32_t)m_B_out.size());
  ic0_msg_reply();

  // clear the didl vectors
  m_B_in.clear();
  m_B_out.clear();
}