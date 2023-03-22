// The class for the Primitive Candid Type: nat64

#pragma once

#include "candid.h"

class CandidTypeNat64 : public CandidTypePrim {
public:
  // Constructors
  CandidTypeNat64();
  CandidTypeNat64(uint64_t *v);
  CandidTypeNat64(const uint64_t v);

  // Destructor
  ~CandidTypeNat64();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error,
                CandidTypeBase *p_expected = nullptr);

  void set_v(const uint64_t &v) {
    m_v = v;
    *m_pv = v;
  }

protected:
  void set_pv(uint64_t *v);
  void initialize(const uint64_t &v);
  void set_datatype();
  void encode_I();
  void encode_M();

  uint64_t m_v;
  uint64_t *m_pv{nullptr};
};