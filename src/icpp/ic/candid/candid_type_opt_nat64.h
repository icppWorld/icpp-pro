// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeOptNat64 : public CandidTypeOptBase {
public:
  // Constructors
  CandidTypeOptNat64();
  CandidTypeOptNat64(const std::optional<uint64_t> v);

  CandidTypeOptNat64(std::optional<uint64_t> *p_v);

  // Destructor
  ~CandidTypeOptNat64();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);

protected:
  void set_pv(std::optional<uint64_t> *v) { m_pv = v; }
  void set_v(const std::optional<uint64_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::optional<uint64_t> m_v;
  std::optional<uint64_t> *m_pv{nullptr};
};