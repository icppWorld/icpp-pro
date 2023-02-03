// The class for the Primitive Candid Type: nat

#pragma once

#include "candid.h"

class CandidTypeNat : public CandidTypePrim {
public:
  // Constructors
  CandidTypeNat();
  CandidTypeNat(const __uint128_t v);

  // Destructor
  ~CandidTypeNat();

protected:
  void initialize(const __uint128_t &v);
  void set_datatype();
  void encode_I();
  void encode_M(const __uint128_t &v);

  __uint128_t m_v;
};