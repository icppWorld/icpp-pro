// The class for the Primitive Candid Type: nat16

#pragma once

#include "candid.h"

class CandidTypeNat16 : public CandidTypePrim {
public:
  // Constructors
  CandidTypeNat16();
  CandidTypeNat16(uint16_t *v);
  CandidTypeNat16(const uint16_t v);

  // Destructor
  ~CandidTypeNat16();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error,
                CandidTypeBase *p_expected = nullptr);

  void set_v(const uint16_t &v) {
    m_v = v;
    *m_pv = v;
  }

protected:
  void set_pv(uint16_t *v);
  void initialize(const uint16_t &v);
  void set_datatype();
  void encode_I();
  void encode_M();

  uint16_t m_v;
  uint16_t *m_pv{nullptr};
};