// Candid deserialization class
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results

#pragma once

#include <string>
#include <vector>

#include "candid.h"

class CandidDeserialize {
public:
  CandidDeserialize();
  CandidDeserialize(const VecBytes &B, std::vector<CandidType> A);
  CandidDeserialize(const std::string hex_string, std::vector<CandidType> A);
  ~CandidDeserialize();

  std::vector<CandidType> get_A() { return m_A; }
  VecBytes get_B() { return m_B; }

  int assert_candid(const std::string &candid_expected,
                    const bool &assert_value);

  static void trap_with_parse_error(const __uint128_t &B_offset_start,
                                    const __uint128_t &B_offset,
                                    const std::string &to_be_parsed,
                                    const std::string &parse_error);

private:
  void deserialize();
  bool deserialize_values_comptype(__uint128_t &B_offset, const size_t i,
                                   __uint128_t &numbytes,
                                   std::string &parse_error);
  bool deserialize_values_primtype(__uint128_t &B_offset, const size_t i,
                                   __uint128_t &numbytes,
                                   std::string &parse_error);

  // The vector with placeholders for the expected arguments of the byte stream coming in
  std::vector<CandidType> m_A;

  // The deserialized type tables
  std::vector<CandidTypeTable> m_typetables;

  // The deserialized argument list
  std::vector<int> m_args_datatypes;

  // The byte stream to be deserialized
  std::string m_hex_string;
  VecBytes m_B;
};