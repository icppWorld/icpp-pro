// The class for the Primitive Candid Type: int16

#pragma once

#include "candid.h"

class CandidTypeInt16 : public CandidTypePrim {
public:
  // Constructors
  CandidTypeInt16();
  CandidTypeInt16(int16_t *v);
  CandidTypeInt16(const int16_t v);

  // Destructor
  ~CandidTypeInt16();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error,
                CandidTypeBase *p_expected = nullptr);

  void set_v(const int16_t &v) {
    m_v = v;
    *m_pv = v;
  }

protected:
  void set_pv(int16_t *v);
  void initialize(const int16_t &v);
  void set_datatype();
  void encode_I();
  void encode_M();

  int16_t m_v;
  int16_t *m_pv{nullptr};
};