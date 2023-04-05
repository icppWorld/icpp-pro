// The class for the Candid Type: vec

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeVecNat::CandidTypeVecNat() : CandidTypeVecBase() {
  std::vector<__uint128_t> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeVecNat::CandidTypeVecNat(std::vector<__uint128_t> *p_v)
    : CandidTypeVecBase() {
  set_pv(p_v);

  const std::vector<__uint128_t> v =
      const_cast<std::vector<__uint128_t> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeVecNat::CandidTypeVecNat(const std::vector<__uint128_t> v)
    : CandidTypeVecBase() {
  set_v(v);
  initialize();
}

CandidTypeVecNat::~CandidTypeVecNat() {}

void CandidTypeVecNat::set_content_type() {
  m_content_type_opcode = CandidOpcode().Nat;
  m_content_type_hex = OpcodeHex().Nat;
  m_content_type_textual = OpcodeTextual().Nat;
}

void CandidTypeVecNat::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(v^N  : vec <datatype>) = leb128(N) M(v : <datatype>)^N

  // encoded size of vec - leb128(N)
  m_M.append_uleb128(__uint128_t(m_v.size()));

  // encoded vec values - M(v : <datatype>)^N
  // Nat:             - M(n : nat<N>)   = i<N>(n)    (Litte Endian)
  for (__uint128_t const &c : m_v) {
    m_M.append_uleb128(c);
  }
}

// Decode the values, starting at & updating offset
bool CandidTypeVecNat::decode_M(VecBytes B, __uint128_t &offset,
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
  __uint128_t v;
  offset_start = offset;
  parse_error = "";
  for (size_t i = 0; i < size_vec; ++i) {
    if (B.parse_uleb128(offset, v, numbytes, parse_error)) {
      std::string to_be_parsed = "Vec: Value for CandidTypeNat";
      CandidDeserialize::trap_with_parse_error(offset_start, offset,
                                               to_be_parsed, parse_error);
    }
    m_v.push_back(v);
  }

  // Fill the user's data placeholder, if a pointer was provided
  if (m_pv) *m_pv = m_v;

  return false;
}