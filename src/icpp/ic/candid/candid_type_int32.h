// The class for the Primitive Candid Type: int32

#pragma once

#include "candid.h"

class CandidTypeInt32 : public CandidTypePrim {
public:
  // Constructors
  CandidTypeInt32();
  // docs start: demo_candid_type_int32
  CandidTypeInt32(int32_t *v);
  CandidTypeInt32(const int32_t v); // docs end: demo_candid_type_int32

  // Destructor
  ~CandidTypeInt32();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  int32_t get_v() { return m_v; }

protected:
  void set_pv(int32_t *v);
  void initialize(const int32_t &v);
  void set_datatype();
  void encode_I();
  void encode_M();

  int32_t m_v;
  int32_t *m_pv{nullptr};
};