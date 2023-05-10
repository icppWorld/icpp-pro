// The class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeVecNat64 : public CandidTypeVecBase {
public:
  // Constructors
  CandidTypeVecNat64();
  CandidTypeVecNat64(const std::vector<uint64_t> v);

  CandidTypeVecNat64(std::vector<uint64_t> *p_v);

  // Destructor
  ~CandidTypeVecNat64();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::vector<uint64_t> get_v() { return m_v; }

protected:
  void set_pv(std::vector<uint64_t> *v) { m_pv = v; }
  void set_v(const std::vector<uint64_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::vector<uint64_t> m_v;
  std::vector<uint64_t> *m_pv{nullptr};
};