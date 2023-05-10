// The class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeVecFloat64 : public CandidTypeVecBase {
public:
  // Constructors
  CandidTypeVecFloat64();
  CandidTypeVecFloat64(const std::vector<double> v);

  CandidTypeVecFloat64(std::vector<double> *p_v);

  // Destructor
  ~CandidTypeVecFloat64();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::vector<double> get_v() { return m_v; }

protected:
  void set_pv(std::vector<double> *v) { m_pv = v; }
  void set_v(const std::vector<double> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::vector<double> m_v;
  std::vector<double> *m_pv{nullptr};
};