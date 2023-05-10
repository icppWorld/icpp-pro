// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeOptInt32 : public CandidTypeOptBase {
public:
  // Constructors
  CandidTypeOptInt32();
  CandidTypeOptInt32(const std::optional<int32_t> v);

  CandidTypeOptInt32(std::optional<int32_t> *p_v);

  // Destructor
  ~CandidTypeOptInt32();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::optional<int32_t> get_v() { return m_v; }

protected:
  void set_pv(std::optional<int32_t> *v) { m_pv = v; }
  void set_v(const std::optional<int32_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::optional<int32_t> m_v;
  std::optional<int32_t> *m_pv{nullptr};
};