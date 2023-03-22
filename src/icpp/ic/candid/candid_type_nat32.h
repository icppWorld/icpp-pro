// The class for the Primitive Candid Type: nat32

#pragma once

#include "candid.h"

class CandidTypeNat32 : public CandidTypePrim {
public:
  // Constructors
  CandidTypeNat32();
  CandidTypeNat32(uint32_t *v);
  CandidTypeNat32(const uint32_t v);

  // Destructor
  ~CandidTypeNat32();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error,
                CandidTypeBase *p_expected = nullptr);

  void set_v(const uint32_t &v) {
    m_v = v;
    *m_pv = v;
  }

protected:
  void set_pv(uint32_t *v);
  void initialize(const uint32_t &v);
  void set_datatype();
  void encode_I();
  void encode_M();

  uint32_t m_v;
  uint32_t *m_pv{nullptr};
};