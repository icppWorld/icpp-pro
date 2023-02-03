// The class for the Primitive Candid Type: empty

#pragma once

#include "candid.h"

class CandidTypeEmpty : public CandidTypePrim {
public:
  // Constructors
  CandidTypeEmpty();

  // Destructor
  ~CandidTypeEmpty();

protected:
  void set_datatype();
  void encode_I();
  void encode_M();
};