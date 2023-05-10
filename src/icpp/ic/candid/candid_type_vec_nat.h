// The class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeVecNat : public CandidTypeVecBase {
public:
  // Constructors
  CandidTypeVecNat();
  CandidTypeVecNat(const std::vector<__uint128_t> v);

  CandidTypeVecNat(std::vector<__uint128_t> *p_v);

  // Destructor
  ~CandidTypeVecNat();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::vector<__uint128_t> get_v() { return m_v; }

protected:
  void set_pv(std::vector<__uint128_t> *v) { m_pv = v; }
  void set_v(const std::vector<__uint128_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::vector<__uint128_t> m_v;
  std::vector<__uint128_t> *m_pv{nullptr};
};