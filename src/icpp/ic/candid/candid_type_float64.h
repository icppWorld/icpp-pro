// The class for the Primitive Candid Type: float64

#pragma once

#include "candid.h"

class CandidTypeFloat64 : public CandidTypePrim {
public:
  // Constructors
  CandidTypeFloat64();
  CandidTypeFloat64(const double v);

  // Destructor
  ~CandidTypeFloat64();

protected:
  void initialize(const double &v);
  void set_datatype();
  void encode_I();
  void encode_M(const double &v);

  double m_v;
};