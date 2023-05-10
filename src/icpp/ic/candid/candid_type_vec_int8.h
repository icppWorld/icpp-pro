// The class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeVecInt8 : public CandidTypeVecBase {
public:
  // Constructors
  CandidTypeVecInt8();
  CandidTypeVecInt8(const std::vector<int8_t> v);

  CandidTypeVecInt8(std::vector<int8_t> *p_v);

  // Destructor
  ~CandidTypeVecInt8();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::vector<int8_t> get_v() { return m_v; }

protected:
  void set_pv(std::vector<int8_t> *v) { m_pv = v; }
  void set_v(const std::vector<int8_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::vector<int8_t> m_v;
  std::vector<int8_t> *m_pv{nullptr};
};