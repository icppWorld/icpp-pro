// The class for the Primitive Candid Type: empty

#pragma once

#include "candid.h"

class CandidTypeEmpty : public CandidTypePrim {
public:
  // Constructors
  CandidTypeEmpty();

  // Destructor
  ~CandidTypeEmpty();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::string get_v() { return "EMPTY_VALUE"; }

protected:
  void set_datatype();
  void encode_I();
  void encode_M();
};