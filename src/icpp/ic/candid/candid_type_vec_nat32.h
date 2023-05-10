// The class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeVecNat32 : public CandidTypeVecBase {
public:
  // Constructors
  CandidTypeVecNat32();
  CandidTypeVecNat32(const std::vector<uint32_t> v);

  CandidTypeVecNat32(std::vector<uint32_t> *p_v);

  // Destructor
  ~CandidTypeVecNat32();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::vector<uint32_t> get_v() { return m_v; }

protected:
  void set_pv(std::vector<uint32_t> *v) { m_pv = v; }
  void set_v(const std::vector<uint32_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::vector<uint32_t> m_v;
  std::vector<uint32_t> *m_pv{nullptr};
};