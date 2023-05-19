// The class for the Primitive Candid Type: float32

#pragma once

#include "candid.h"

class CandidTypeFloat32 : public CandidTypePrim {
public:
  // Constructors
  CandidTypeFloat32();
  // docs start: demo_candid_type_float32
  CandidTypeFloat32(float *v);
  CandidTypeFloat32(const float v); // docs end: demo_candid_type_float32

  // Destructor
  ~CandidTypeFloat32();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  float get_v() { return m_v; }

protected:
  void set_pv(float *v);
  void initialize(const float &v);
  void set_datatype();
  void encode_I();
  void encode_M();

  float m_v;
  float *m_pv{nullptr};
};