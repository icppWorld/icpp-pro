// The class for the Candid Type: vec

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeVecPrincipal::CandidTypeVecPrincipal() : CandidTypeVecBase() {
  std::vector<std::string> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeVecPrincipal::CandidTypeVecPrincipal(std::vector<std::string> *p_v)
    : CandidTypeVecBase() {
  set_pv(p_v);

  const std::vector<std::string> v =
      const_cast<std::vector<std::string> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeVecPrincipal::CandidTypeVecPrincipal(const std::vector<std::string> v)
    : CandidTypeVecBase() {
  set_v(v);
  initialize();
}

CandidTypeVecPrincipal::~CandidTypeVecPrincipal() {}

void CandidTypeVecPrincipal::set_content_type() {
  m_content_type_opcode = CandidOpcode().Principal;
  m_content_type_hex = OpcodeHex().Principal;
  m_content_type_textual = OpcodeTextual().Principal;
}

void CandidTypeVecPrincipal::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(v^N  : vec <datatype>) = leb128(N) M(v : <datatype>)^N

  // encoded size of vec - leb128(N)
  m_M.append_uleb128(__uint128_t(m_v.size()));

  // encoded vec values - M(v : <datatype>)^N
  for (std::string const &s : m_v) {
    CandidTypePrincipal c{s};
    VecBytes m = c.get_M();
    m_M.append_bytes(m.vec_uint8_t().data(), m.size());
  }
}

// Decode the values, starting at & updating offset
bool CandidTypeVecPrincipal::decode_M(VecBytes B, __uint128_t &offset,
                                      std::string &parse_error) {
  // get size of vec - leb128(N)
  __uint128_t offset_start = offset;
  __uint128_t numbytes;
  parse_error = "";
  __uint128_t size_vec;
  if (B.parse_uleb128(offset, size_vec, numbytes, parse_error)) {
    std::string to_be_parsed = "Size of vec- leb128(N)";
    CandidDeserialize::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                             parse_error);
  }

  m_v.clear();
  offset_start = offset;
  parse_error = "";
  CandidTypePrincipal c{""}; // dummy so we can use it's decode_M
  for (size_t i = 0; i < size_vec; ++i) {
    if (c.decode_M(B, offset, parse_error)) {
      std::string to_be_parsed = "Vec: Value for CandidTypePrincipal";
      CandidDeserialize::trap_with_parse_error(offset_start, offset,
                                               to_be_parsed, parse_error);
    }
    m_v.push_back(c.get_v());
  }

  // Fill the user's data placeholder, if a pointer was provided
  if (m_pv) *m_pv = m_v;

  return false;
}