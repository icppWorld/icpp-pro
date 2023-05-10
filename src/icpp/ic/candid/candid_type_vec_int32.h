// The class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeVecInt32 : public CandidTypeVecBase {
public:
  // Constructors
  CandidTypeVecInt32();
  CandidTypeVecInt32(const std::vector<int32_t> v);

  CandidTypeVecInt32(std::vector<int32_t> *p_v);

  // Destructor
  ~CandidTypeVecInt32();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::vector<int32_t> get_v() { return m_v; }

protected:
  void set_pv(std::vector<int32_t> *v) { m_pv = v; }
  void set_v(const std::vector<int32_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::vector<int32_t> m_v;
  std::vector<int32_t> *m_pv{nullptr};
};