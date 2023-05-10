// The class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeVecNat8 : public CandidTypeVecBase {
public:
  // Constructors
  CandidTypeVecNat8();
  CandidTypeVecNat8(const std::vector<uint8_t> v);

  CandidTypeVecNat8(std::vector<uint8_t> *p_v);

  // Destructor
  ~CandidTypeVecNat8();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::vector<uint8_t> get_v() { return m_v; }

protected:
  void set_pv(std::vector<uint8_t> *v) { m_pv = v; }
  void set_v(const std::vector<uint8_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::vector<uint8_t> m_v;
  std::vector<uint8_t> *m_pv{nullptr};
};