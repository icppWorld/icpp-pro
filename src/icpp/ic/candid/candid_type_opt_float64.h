// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeOptFloat64 : public CandidTypeOptBase {
public:
  // Constructors
  CandidTypeOptFloat64();
  CandidTypeOptFloat64(const std::optional<double> v);

  CandidTypeOptFloat64(std::optional<double> *p_v);

  // Destructor
  ~CandidTypeOptFloat64();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::optional<double> get_v() { return m_v; }

protected:
  void set_pv(std::optional<double> *v) { m_pv = v; }
  void set_v(const std::optional<double> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::optional<double> m_v;
  std::optional<double> *m_pv{nullptr};
};