// The class for the Primitive Candid Type: int16

#pragma once

#include "candid.h"

class CandidTypeInt16 : public CandidTypePrim {
public:
  // Constructors
  CandidTypeInt16();
  // docs start: demo_candid_type_int16
  CandidTypeInt16(int16_t *v);
  CandidTypeInt16(const int16_t v); // docs end: demo_candid_type_int16

  // Destructor
  ~CandidTypeInt16();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  int16_t get_v() { return m_v; }

protected:
  void set_pv(int16_t *v);
  void initialize(const int16_t &v);
  void set_datatype();
  void encode_I();
  void encode_M();

  int16_t m_v;
  int16_t *m_pv{nullptr};
};