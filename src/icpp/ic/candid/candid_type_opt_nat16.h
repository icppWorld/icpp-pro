// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeOptNat16 : public CandidTypeOptBase {
public:
  // Constructors
  CandidTypeOptNat16();
  CandidTypeOptNat16(const std::optional<uint16_t> v);

  CandidTypeOptNat16(std::optional<uint16_t> *p_v);

  // Destructor
  ~CandidTypeOptNat16();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::optional<uint16_t> get_v() { return m_v; }

protected:
  void set_pv(std::optional<uint16_t> *v) { m_pv = v; }
  void set_v(const std::optional<uint16_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::optional<uint16_t> m_v;
  std::optional<uint16_t> *m_pv{nullptr};
};