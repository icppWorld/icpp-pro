// The class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeVecNat16 : public CandidTypeVecBase {
public:
  // Constructors
  CandidTypeVecNat16();
  CandidTypeVecNat16(const std::vector<uint16_t> v);

  CandidTypeVecNat16(std::vector<uint16_t> *p_v);

  // Destructor
  ~CandidTypeVecNat16();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::vector<uint16_t> get_v() { return m_v; }

protected:
  void set_pv(std::vector<uint16_t> *v) { m_pv = v; }
  void set_v(const std::vector<uint16_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::vector<uint16_t> m_v;
  std::vector<uint16_t> *m_pv{nullptr};
};