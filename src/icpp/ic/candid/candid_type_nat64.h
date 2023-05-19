// The class for the Primitive Candid Type: nat64

#pragma once

#include "candid.h"

class CandidTypeNat64 : public CandidTypePrim {
public:
  // Constructors
  CandidTypeNat64();
  // docs start: demo_candid_type_nat64
  CandidTypeNat64(uint64_t *v);
  CandidTypeNat64(const uint64_t v); // docs end: demo_candid_type_nat64

  // Destructor
  ~CandidTypeNat64();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  uint64_t get_v() { return m_v; }

protected:
  void set_pv(uint64_t *v);
  void initialize(const uint64_t &v);
  void set_datatype();
  void encode_I();
  void encode_M();

  uint64_t m_v;
  uint64_t *m_pv{nullptr};
};