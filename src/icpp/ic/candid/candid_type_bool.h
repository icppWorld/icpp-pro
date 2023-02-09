// The class for the Primitive Candid Type: bool

#pragma once

#include "candid.h"

class CandidTypeBool : public CandidTypePrim {
public:
  // Constructors
  CandidTypeBool();
  CandidTypeBool(bool *v);
  CandidTypeBool(const bool v);

  // Destructor
  ~CandidTypeBool();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error,
                CandidTypeBase *p_expected = nullptr);

  void set_v(const bool &v) {
    m_v = v;
    *m_pv = v;
  }

protected:
  void set_pv(bool *v);
  void initialize(const bool &v);
  void set_datatype();
  void encode_I();
  void encode_M(const bool &v);

  bool m_v;
  bool *m_pv{nullptr};
};