// The class for the Primitive Candid Type: text

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstring>

#include "candid.h"

class CandidTypePrincipal : public CandidTypePrim {
public:
  // Constructors
  CandidTypePrincipal();
  CandidTypePrincipal(const char *c);
  CandidTypePrincipal(const std::string v);

  CandidTypePrincipal(std::string *v);

  // Destructor
  ~CandidTypePrincipal();

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
  void encode_M();
  void bytes_from_string();
  std::string string_from_bytes(std::vector<uint8_t> &data_bytes);

  void make_ascii_uppercase(std::string &s) {
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
  }
  void make_ascii_lowercase(std::string &s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  }
  void ungroup(std::string &s);
  void group(std::string &s);
  std::vector<uint8_t> base32_decode(std::string &s);
  std::string base32_encode(std::vector<uint8_t> &bytes);
  void pad(std::string &s);
  void nopad(std::string &s);
  std::array<uint8_t, 4> crc32(std::vector<uint8_t> data_bytes);

  std::string m_v;    // text representation of Principal
  VecBytes m_v_bytes; // byte representation of Principal
  std::string *m_pv{nullptr};
};