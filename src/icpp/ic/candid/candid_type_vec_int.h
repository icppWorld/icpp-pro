// The class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeVecInt : public CandidTypeVecBase {
public:
  // Constructors
  CandidTypeVecInt();
  CandidTypeVecInt(const std::vector<__int128_t> v);

  CandidTypeVecInt(std::vector<__int128_t> *p_v);

  // Destructor
  ~CandidTypeVecInt();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::vector<__int128_t> get_v() { return m_v; }

protected:
  void set_pv(std::vector<__int128_t> *v) { m_pv = v; }
  void set_v(const std::vector<__int128_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::vector<__int128_t> m_v;
  std::vector<__int128_t> *m_pv{nullptr};
};