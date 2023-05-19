// The class for the Primitive Candid Type: text

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeText : public CandidTypePrim {
public:
  // Constructors
  CandidTypeText();
  CandidTypeText(const char *c);
  // docs start: demo_candid_type_text
  CandidTypeText(std::string *v);
  CandidTypeText(const std::string v); // docs end: demo_candid_type_text

  // Destructor
  ~CandidTypeText();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::string get_v() { return m_v; }

protected:
  void set_pv(std::string *v);
  void initialize(const std::string &v);
  void set_datatype();
  void encode_I();
  void encode_M();

  std::string m_v;
  std::string *m_pv{nullptr};
};