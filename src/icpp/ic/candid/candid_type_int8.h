// The class for the Primitive Candid Type: int8

#pragma once

#include "candid.h"

class CandidTypeInt8 : public CandidTypePrim {
public:
  // Constructors
  CandidTypeInt8();
  // docs start: demo_candid_type_int8
  CandidTypeInt8(int8_t *v);
  CandidTypeInt8(const int8_t v); // docs end: demo_candid_type_int8

  // Destructor
  ~CandidTypeInt8();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  int8_t get_v() { return m_v; }

protected:
  void set_pv(int8_t *v);
  void initialize(const int8_t &v);
  void set_datatype();
  void encode_I();
  void encode_M();

  int8_t m_v;
  int8_t *m_pv{nullptr};
};