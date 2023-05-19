// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeOptNat8 : public CandidTypeOptBase {
public:
  // Constructors
  CandidTypeOptNat8();
  // clang-format off
  // docs start: demo_candid_type_opt
  CandidTypeOptNat8(std::optional<uint8_t> *v);
  CandidTypeOptNat8(const std::optional<uint8_t> v); // docs end: demo_candid_type_opt
  // clang-format on

  // Destructor
  ~CandidTypeOptNat8();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::optional<uint8_t> get_v() { return m_v; }

protected:
  void set_pv(std::optional<uint8_t> *v) { m_pv = v; }
  void set_v(const std::optional<uint8_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::optional<uint8_t> m_v;
  std::optional<uint8_t> *m_pv{nullptr};
};