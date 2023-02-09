// The class for the Primitive Candid Type: int

#pragma once

#include "candid.h"

class CandidTypeInt : public CandidTypePrim {
public:
  // Constructors
  CandidTypeInt();
  CandidTypeInt(__int128_t *v);
  CandidTypeInt(const __int128_t &v);

  // Destructor
  ~CandidTypeInt();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error,
                CandidTypeBase *p_expected = nullptr);

  void set_v(const __int128_t &v) {
    m_v = v;
    *m_pv = v;
  }

protected:
  void set_pv(__int128_t *v);
  void initialize(const __int128_t &v);
  void set_datatype();
  void encode_I();
  void encode_M(const __int128_t &v);

  __int128_t m_v;
  __int128_t *m_pv{nullptr};
};