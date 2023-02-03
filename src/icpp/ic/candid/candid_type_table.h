// The candid type table of a Candid variable
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results

#pragma once

#include <string>
#include <vector>

#include "candid.h"

class CandidTypeTable {
public:
  CandidTypeTable();
  CandidTypeTable(const VecBytes &B, __uint128_t &B_offset);
  ~CandidTypeTable();

  VecBytes get_B() { return m_B; }
  int get_opcode() { return m_opcode; }
  CandidType *get_candidType_ptr() { return &m_c; }

private:
  void deserialize(__uint128_t &B_offset);

  // The deserialized type table
  int m_opcode;
  CandidType m_c;

  // The byte stream to be deserialized
  __uint128_t m_B_offset;
  VecBytes m_B;
};