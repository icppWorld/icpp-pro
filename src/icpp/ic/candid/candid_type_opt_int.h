// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeOptInt : public CandidTypeOptBase {
public:
  // Constructors
  CandidTypeOptInt();
  CandidTypeOptInt(const std::optional<__int128_t> v);

  CandidTypeOptInt(std::optional<__int128_t> *p_v);

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