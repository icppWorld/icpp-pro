// The class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeVecText : public CandidTypeVecBase {
public:
  // Constructors
  CandidTypeVecText();
  CandidTypeVecText(const std::vector<std::string> v);

  CandidTypeVecText(std::vector<std::string> *p_v);

  // Destructor
  ~CandidTypeVecText();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::vector<std::string> get_v() { return m_v; }

protected:
  void set_pv(std::vector<std::string> *v) { m_pv = v; }
  void set_v(const std::vector<std::string> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::vector<std::string> m_v;
  std::vector<std::string> *m_pv{nullptr};
};