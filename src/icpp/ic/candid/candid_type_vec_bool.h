// The class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeVecBool : public CandidTypeVecBase {
public:
  // Constructors
  CandidTypeVecBool();
  CandidTypeVecBool(const std::vector<bool> v);

  CandidTypeVecBool(std::vector<bool> *p_v);

  // Destructor
  ~CandidTypeVecBool();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::vector<bool> get_v() { return m_v; }

protected:
  void set_pv(std::vector<bool> *v) { m_pv = v; }
  void set_v(const std::vector<bool> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::vector<bool> m_v;
  std::vector<bool> *m_pv{nullptr};
};