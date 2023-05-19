// The class for the Primitive Candid Type: nat8

#pragma once

#include "candid.h"

class CandidTypeNat8 : public CandidTypePrim {
public:
  // Constructors
  CandidTypeNat8();
  // docs start: demo_candid_type_nat8
  CandidTypeNat8(uint8_t *v);
  CandidTypeNat8(const uint8_t v); // docs end: demo_candid_type_nat8

  // Destructor
  ~CandidTypeNat8();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  uint8_t get_v() { return m_v; }

protected:
  void set_pv(uint8_t *v);
  void initialize(const uint8_t &v);
  void set_datatype();
  void encode_I();
  void encode_M();

  uint8_t m_v;
  uint8_t *m_pv{nullptr};
};