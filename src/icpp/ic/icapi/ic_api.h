// API functions to be called by a C++ application to interact with ic0
// functions
//
// References:
//  -
//  https://smartcontracts.org/docs/interface-spec/index.html#system-api-imports
//  - https://github.com/dfinity/candid/blob/master/spec/Candid.md#binary-format

#pragma once

#include <string>

#include "candid.h"
#include "vec_bytes.h"

class IC_API {
public:
  IC_API();
  IC_API(const bool &debug_print);

  ~IC_API();

  // Static utility functions
  static void debug_print(const char *message);
  static void debug_print(const std::string &s);
  static void trap(const char *message);
  static void trap(const std::string &s);

  uint64_t time();

  // Receive things from the wire in candid format
  void from_wire(std::vector<CandidType> A);
  void from_wire(CandidType c);

  // Send things back over the wire in candid format
  void to_wire(const CandidType &arg_out);
  void to_wire(const std::vector<CandidType> &args_out);

private:
  bool m_debug_print;
  VecBytes m_B_in;
  VecBytes m_B_out;
  void msg_reply();
};