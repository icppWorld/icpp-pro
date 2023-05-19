// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeOptPrincipal : public CandidTypeOptBase {
public:
  // Constructors
  CandidTypeOptPrincipal();
  // clang-format off
  // docs start: demo_candid_type_opt
  CandidTypeOptPrincipal(std::optional<std::string> *p_v);
  CandidTypeOptPrincipal(const std::optional<std::string> v); // docs end: demo_candid_type_opt
  // clang-format on

  // Destructor
  ~CandidTypeOptPrincipal();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::optional<std::string> get_v() { return m_v; }

protected:
  void set_pv(std::optional<std::string> *v) { m_pv = v; }
  void set_v(const std::optional<std::string> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::optional<std::string> m_v;
  std::optional<std::string> *m_pv{nullptr};
};