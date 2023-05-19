// The class for the Primitive Candid Type: float64

#pragma once

#include "candid.h"

class CandidTypeFloat64 : public CandidTypePrim {
public:
  // Constructors
  CandidTypeFloat64();
  // docs start: demo_candid_type_float64
  CandidTypeFloat64(double *v);
  CandidTypeFloat64(const double v); // docs end: demo_candid_type_float64

  // Destructor
  ~CandidTypeFloat64();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  double get_v() { return m_v; }

protected:
  void set_pv(double *v);
  void initialize(const double &v);
  void set_datatype();
  void encode_I();
  void encode_M();

  double m_v;
  double *m_pv{nullptr};
};