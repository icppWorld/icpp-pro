// The class for the Primitive Candid Type: int64

#include "candid.h"

#include "candid_opcode.h"

CandidTypeInt64::CandidTypeInt64() : CandidTypePrim() { initialize(0); }

// This constructor allows for setting the value during Deserialization
CandidTypeInt64::CandidTypeInt64(int64_t *p_v) : CandidTypePrim() {
  set_pv(p_v);

  const int64_t v = const_cast<int64_t &>(*p_v);
  initialize(v);
}

// This constructor is only for encoding
CandidTypeInt64::CandidTypeInt64(const int64_t v) : CandidTypePrim() {
  initialize(v);
}

CandidTypeInt64::~CandidTypeInt64() {}

// Initialize things
void CandidTypeInt64::initialize(const int64_t &v) {
  m_v = v;
  set_datatype();
  encode_I();
  encode_M();
}

// pointer to data in caller, for storing decoded value
void CandidTypeInt64::set_pv(int64_t *v) { m_pv = v; }

void CandidTypeInt64::set_datatype() {
  m_datatype_opcode = CandidOpcode().Int64;
  m_datatype_hex = OpcodeHex().Int64;
  m_datatype_textual = OpcodeTextual().Int64;
}

void CandidTypeInt64::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
  m_I.append_byte((std::byte)m_datatype_hex);
}

void CandidTypeInt64::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(n : int<N>)   = i<N>(signed_N^-1(i))    (Litte Endian)
  m_M.append_int_fixed_width(m_v);
}

// Decode the values, starting at & updating offset
bool CandidTypeInt64::decode_M(VecBytes B, __uint128_t &offset,
                               std::string &parse_error,
                               CandidTypeBase *p_expected) {

  __uint128_t offset_start = offset;
  parse_error = "";
  if (B.parse_int_fixed_width(offset, m_v, parse_error)) {
    std::string to_be_parsed = "Value for CandidTypeInt64";
    CandidDeserialize::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                             parse_error);
  }

  // Fill the user's data placeholder, if a pointer was provided
  if (m_pv) *m_pv = m_v;

  return false;
}