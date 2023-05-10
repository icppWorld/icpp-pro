// The class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeVecInt64 : public CandidTypeVecBase {
public:
  // Constructors
  CandidTypeVecInt64();
  CandidTypeVecInt64(const std::vector<int64_t> v);

  CandidTypeVecInt64(std::vector<int64_t> *p_v);

  // Destructor
  ~CandidTypeVecInt64();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::vector<int64_t> get_v() { return m_v; }

protected:
  void set_pv(std::vector<int64_t> *v) { m_pv = v; }
  void set_v(const std::vector<int64_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::vector<int64_t> m_v;
  std::vector<int64_t> *m_pv{nullptr};
};