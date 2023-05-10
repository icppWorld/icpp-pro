// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeOptNat : public CandidTypeOptBase {
public:
  // Constructors
  CandidTypeOptNat();
  CandidTypeOptNat(const std::optional<__uint128_t> v);

  CandidTypeOptNat(std::optional<__uint128_t> *p_v);

  // Destructor
  ~CandidTypeOptNat();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::optional<__uint128_t> get_v() { return m_v; }

protected:
  void set_pv(std::optional<__uint128_t> *v) { m_pv = v; }
  void set_v(const std::optional<__uint128_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::optional<__uint128_t> m_v;
  std::optional<__uint128_t> *m_pv{nullptr};
};