// The class for the Primitive Candid Type: null

#pragma once

#include "candid.h"

class CandidTypeNull : public CandidTypePrim {
public:
  // Constructors
  CandidTypeNull();

  // Destructor
  ~CandidTypeNull();

  std::string get_v() { return "NULL_VALUE"; }

protected:
  void set_datatype();
  void encode_I();
  void encode_M();
};