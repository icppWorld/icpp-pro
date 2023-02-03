// The class for the Primitive Candid Type: null

#pragma once

#include "candid.h"

class CandidTypeNull : public CandidTypePrim {
public:
  // Constructors
  CandidTypeNull();

  // Destructor
  ~CandidTypeNull();

protected:
  void set_datatype();
  void encode_I();
  void encode_M();
};