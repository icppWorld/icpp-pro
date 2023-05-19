// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeOptInt : public CandidTypeOptBase {
public:
  // Constructors
  CandidTypeOptInt();
  // clang-format off
  // docs start: demo_candid_type_opt
  CandidTypeOptInt(std::optional<__int128_t> *v);
  CandidTypeOptInt(const std::optional<__int128_t> v); // docs end: demo_candid_type_opt
  // clang-format on

  // Destructor
  ~CandidTypeOptInt();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::optional<__int128_t> get_v() { return m_v; }

protected:
  void set_pv(std::optional<__int128_t> *v) { m_pv = v; }
  void set_v(const std::optional<__int128_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::optional<__int128_t> m_v;
  std::optional<__int128_t> *m_pv{nullptr};
};