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
#include "candid_type_principal.h"
#include "vec_bytes.h"

class IC_API {
public:
  IC_API();
  // docs start: ac_api
  IC_API(const bool &debug_print); // docs end: ac_api

  ~IC_API();

  // docs start: demo_debug_print
  static void debug_print(const char *msg);
  static void debug_print(const std::string &msg); // docs end: demo_debug_print

  // docs start: demo_trap
  static void trap(const char *msg);
  static void trap(const std::string &msg); // docs end: demo_trap

  // docs start: demo_time
  uint64_t time(); // docs end: demo_time

  // docs start: get_caller
  CandidTypePrincipal get_caller(); // docs end: get_caller

  // Receive things from the wire in candid format
  // docs start: from_wire
  void from_wire();
  void from_wire(CandidType arg_in);
  void from_wire(std::vector<CandidType> args_in); // docs end: from_wire

  // Convert string to __uint128_t & __int128_t
  // clang-format off
  // docs start: string_to_128
  static std::optional<__uint128_t> string_to_uint128_t(const std::string &str);
  static std::optional<__int128_t> string_to_int128_t(const std::string &str); // docs end: string_to_128
  // clang-format on

  // Convert __uint128_t & __int128_t to std::string
  // docs start: to_string_128
  static std::string to_string_128(__uint128_t v);
  static std::string to_string_128(__int128_t v); // docs end: to_string_128

  // Send things back over the wire in candid format
  // docs start: to_wire
  void to_wire();
  void to_wire(const CandidType &arg_out);
  void to_wire(const std::vector<CandidType> &args_out); // docs end: to_wire

private:
  bool m_debug_print;
  VecBytes m_B_in;
  VecBytes m_B_out;
  CandidTypePrincipal m_caller;
  void msg_reply();

  bool m_called_from_wire{false};
  bool m_called_to_wire{false};
};