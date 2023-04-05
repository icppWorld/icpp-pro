// The base class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeVecBase : public CandidTypeBase {
public:
  // Constructors
  CandidTypeVecBase();

  // Destructor
  ~CandidTypeVecBase();

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);

protected:
  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};