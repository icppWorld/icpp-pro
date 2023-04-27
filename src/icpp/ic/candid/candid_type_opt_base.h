// The base class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeOptBase : public CandidTypeBase {
public:
  // Constructors
  CandidTypeOptBase();

  // Destructor
  ~CandidTypeOptBase();

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);

protected:
  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};