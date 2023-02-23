// The class for the Primitive Candid Type: reserved
#pragma once

#include "candid.h"

class CandidTypeReserved : public CandidTypePrim {
public:
  // Constructors
  CandidTypeReserved();

  // Destructor
  ~CandidTypeReserved();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error,
                CandidTypeBase *p_expected = nullptr);

protected:
  void set_datatype();
  void encode_I();
  void encode_M();
};