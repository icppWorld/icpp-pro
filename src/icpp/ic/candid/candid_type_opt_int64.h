// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeOptInt64 : public CandidTypeOptBase {
public:
  // Constructors
  CandidTypeOptInt64();
  CandidTypeOptInt64(const std::optional<int64_t> v);

  CandidTypeOptInt64(std::optional<int64_t> *p_v);

  // Destructor
  ~CandidTypeOptInt64();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);

protected:
  void set_pv(std::optional<int64_t> *v) { m_pv = v; }
  void set_v(const std::optional<int64_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::optional<int64_t> m_v;
  std::optional<int64_t> *m_pv{nullptr};
};