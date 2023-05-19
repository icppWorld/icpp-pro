// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeOptInt64 : public CandidTypeOptBase {
public:
  // Constructors
  CandidTypeOptInt64();
  // clang-format off
  // docs start: demo_candid_type_opt
  CandidTypeOptInt64(std::optional<int64_t> *v);
  CandidTypeOptInt64(const std::optional<int64_t> v); // docs end: demo_candid_type_opt
  // clang-format on

  // Destructor
  ~CandidTypeOptInt64();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::optional<int64_t> get_v() { return m_v; }

protected:
  void set_pv(std::optional<int64_t> *v) { m_pv = v; }
  void set_v(const std::optional<int64_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::optional<int64_t> m_v;
  std::optional<int64_t> *m_pv{nullptr};
};