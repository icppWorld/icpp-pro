// The class for the Primitive Candid Type: bool

#pragma once

#include "candid.h"

class CandidTypeBool : public CandidTypePrim {
public:
  // Constructors
  CandidTypeBool();
  // docs start: demo_candid_type_bool
  CandidTypeBool(bool *v);
  CandidTypeBool(const bool v); // docs end: demo_candid_type_bool

  // Destructor
  ~CandidTypeBool();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  bool get_v() { return m_v; }

protected:
  void set_pv(bool *v);
  void initialize(const bool &v);
  void set_datatype();
  void encode_I();
  void encode_M();

  bool m_v;
  bool *m_pv{nullptr};
};