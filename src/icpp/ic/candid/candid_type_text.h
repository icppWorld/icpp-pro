// The class for the Primitive Candid Type: text

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeText : public CandidTypePrim {
public:
  // Constructors
  CandidTypeText();
  CandidTypeText(const char *c);
  CandidTypeText(const std::string v);

  CandidTypeText(std::string *v);

  // Destructor
  ~CandidTypeText();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error,
                CandidTypeBase *p_expected = nullptr);

  void set_v(const std::string &v) {
    m_v = v;
    *m_pv = v;
  }

protected:
  void set_pv(std::string *v);
  void initialize(const std::string &v);
  void set_datatype();
  void encode_I();
  void encode_M(const std::string &v);

  std::string m_v;
  std::string *m_pv{nullptr};
};