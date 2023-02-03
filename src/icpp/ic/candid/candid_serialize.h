// Candid serialization class
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results

#pragma once

#include <string>
#include <vector>

#include "candid.h"

class CandidSerialize {
public:
  CandidSerialize(const CandidType &a);
  CandidSerialize(const std::vector<CandidType> &A);
  CandidSerialize();
  ~CandidSerialize();

  std::vector<CandidType> get_A() { return m_A; }
  VecBytes get_B() { return m_B; }
  __uint128_t get_num_typetables() { return m_num_typetables; }

  int assert_candid(const std::string &candid_expected,
                    const bool &assert_value);

private:
  void serialize();

  // The argument list to be serialized
  std::vector<CandidType> m_A;

  // The serialized byte stream
  VecBytes m_B;

  // For the <comptype> arguments with a unique type table
  __uint128_t m_num_typetables;
  std::vector<__uint128_t> m_type_table_A_index; // Index into m_A
  std::vector<__uint128_t> m_type_table_index;   // unique type table index
};