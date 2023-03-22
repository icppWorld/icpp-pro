// The class for the Primitive Candid Type: int64

#pragma once

#include "candid.h"

class CandidTypeInt64 : public CandidTypePrim {
public:
  // Constructors
  CandidTypeInt64();
  CandidTypeInt64(int64_t *v);
  CandidTypeInt64(const int64_t v);

  // Destructor
  ~CandidTypeInt64();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error,
                CandidTypeBase *p_expected = nullptr);

  void set_v(const int64_t &v) {
    m_v = v;
    *m_pv = v;
  }

protected:
  void set_pv(int64_t *v);
  void initialize(const int64_t &v);
  void set_datatype();
  void encode_I();
  void encode_M();

  int64_t m_v;
  int64_t *m_pv{nullptr};
};