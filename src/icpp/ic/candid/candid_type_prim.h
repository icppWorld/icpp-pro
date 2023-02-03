// The class for the Candid <primtype>

#pragma once

#include "candid.h"

class CandidTypePrim : public CandidTypeBase {
public:
  // Constructors
  CandidTypePrim();

  // Destructor
  ~CandidTypePrim();

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error) {
    return false; // type table for Primitives is empty
  }

protected:
  void encode_T() { m_T.clear(); } // type table for Primitives is empty
};