// The class for the Primitive Candid Type: text

#include "candid.h"
#include "candid_opcode.h"
#include "pro.h"

#include <cassert>

#include "cppcodec/base32_rfc4648.hpp"
#include "hash-library/crc32.h"

#include "ic_api.h"

CandidTypePrincipal::CandidTypePrincipal() : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypePrincipal::CandidTypePrincipal(std::string *p_v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypePrincipal::CandidTypePrincipal(const char *c) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}
CandidTypePrincipal::CandidTypePrincipal(const std::string v)
    : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypePrincipal::~CandidTypePrincipal() {}

void CandidTypePrincipal::initialize(const std::string &v) {}

void CandidTypePrincipal::set_pv(std::string *v) {}

void CandidTypePrincipal::set_datatype() {}

void CandidTypePrincipal::encode_I() {}

void CandidTypePrincipal::encode_M() {}

bool CandidTypePrincipal::decode_M(VecBytes B, __uint128_t &offset,
                                   std::string &parse_error) {

  return false;
}

void CandidTypePrincipal::bytes_from_string() {}

std::string
CandidTypePrincipal::string_from_bytes(std::vector<uint8_t> &data_bytes) {
  return "";
}

void CandidTypePrincipal::ungroup(std::string &s) {}

void CandidTypePrincipal::group(std::string &s) {}

std::vector<uint8_t> CandidTypePrincipal::base32_decode(std::string &s) {
  // cppcodec requires padding with "="
  pad(s);
  return cppcodec::base32_rfc4648::decode(s);
}

std::string CandidTypePrincipal::base32_encode(std::vector<uint8_t> &bytes) {
  std::string s = cppcodec::base32_rfc4648::encode(bytes);
  nopad(s);
  return s;
}

void CandidTypePrincipal::pad(std::string &s) {}

void CandidTypePrincipal::nopad(std::string &s) {}

std::array<uint8_t, 4>
CandidTypePrincipal::crc32(std::vector<uint8_t> data_bytes) {
  CRC32 crc32;
  std::string crc32_data_str = crc32(data_bytes.data(), data_bytes.size());
  std::array<uint8_t, 4> crc_data;
  crc32.getHash(crc_data.data());
  return crc_data;
}