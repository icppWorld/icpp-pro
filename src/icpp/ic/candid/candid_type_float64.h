// The class for the Primitive Candid Type: float64

#pragma once

#include "candid.h"

class CandidTypeFloat64 : public CandidTypePrim {
public:
  // Constructors
  CandidTypeFloat64();
  CandidTypeFloat64(double *v);
  CandidTypeFloat64(const double v);

  // Destructor
  ~CandidTypeFloat64();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error,
                CandidTypeBase *p_expected = nullptr);

  void set_v(const double &v) {
    m_v = v;
    *m_pv = v;
  }

protected:
  void set_pv(double *v);
  void initialize(const double &v);
  void set_datatype();
  void encode_I();
  void encode_M();

  double m_v;
  double *m_pv{nullptr};
};