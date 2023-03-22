// The class for the Primitive Candid Type: nat

#pragma once

#include "candid.h"

class CandidTypeNat : public CandidTypePrim {
public:
  // Constructors
  CandidTypeNat();
  CandidTypeNat(__uint128_t *v);
  CandidTypeNat(const __uint128_t v);

  // Destructor
  ~CandidTypeNat();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error,
                CandidTypeBase *p_expected = nullptr);

  void set_v(const __uint128_t &v) {
    m_v = v;
    *m_pv = v;
  }

protected:
  void set_pv(__uint128_t *v);
  void initialize(const __uint128_t &v);
  void set_datatype();
  void encode_I();
  void encode_M();

  __uint128_t m_v;
  __uint128_t *m_pv{nullptr};
};