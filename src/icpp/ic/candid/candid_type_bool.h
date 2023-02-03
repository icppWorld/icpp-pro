// The class for the Primitive Candid Type: bool

#pragma once

#include "candid.h"

class CandidTypeBool : public CandidTypePrim {
public:
  // Constructors
  CandidTypeBool();
  CandidTypeBool(const bool v);

  // Destructor
  ~CandidTypeBool();

protected:
  void initialize(const bool &v);
  void set_datatype();
  void encode_I();
  void encode_M(const bool &v);

  bool m_v;
};