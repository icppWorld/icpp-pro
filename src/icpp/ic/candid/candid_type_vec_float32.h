// The class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeVecFloat32 : public CandidTypeVecBase {
public:
  // Constructors
  CandidTypeVecFloat32();
  CandidTypeVecFloat32(const std::vector<float> v);

  CandidTypeVecFloat32(std::vector<float> *p_v);

  // Destructor
  ~CandidTypeVecFloat32();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::vector<float> get_v() { return m_v; }

protected:
  void set_pv(std::vector<float> *v) { m_pv = v; }
  void set_v(const std::vector<float> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::vector<float> m_v;
  std::vector<float> *m_pv{nullptr};
};