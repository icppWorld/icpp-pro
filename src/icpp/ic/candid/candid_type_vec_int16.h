// The class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeVecInt16 : public CandidTypeVecBase {
public:
  // Constructors
  CandidTypeVecInt16();
  CandidTypeVecInt16(const std::vector<int16_t> v);

  CandidTypeVecInt16(std::vector<int16_t> *p_v);

  // Destructor
  ~CandidTypeVecInt16();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::vector<int16_t> get_v() { return m_v; }

protected:
  void set_pv(std::vector<int16_t> *v) { m_pv = v; }
  void set_v(const std::vector<int16_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::vector<int16_t> m_v;
  std::vector<int16_t> *m_pv{nullptr};
};