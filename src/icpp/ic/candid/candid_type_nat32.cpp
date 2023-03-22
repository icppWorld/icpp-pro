// The class for the Primitive Candid Type: nat32

#include "candid.h"

#include "candid_opcode.h"

CandidTypeNat32::CandidTypeNat32() : CandidTypePrim() { initialize(0); }

// This constructor allows for setting the value during Deserialization
CandidTypeNat32::CandidTypeNat32(uint32_t *p_v) : CandidTypePrim() {
  set_pv(p_v);

  const uint32_t v = const_cast<uint32_t &>(*p_v);
  initialize(v);
}

// This constructor is only for encoding
CandidTypeNat32::CandidTypeNat32(const uint32_t v) : CandidTypePrim() {
  initialize(v);
}

CandidTypeNat32::~CandidTypeNat32() {}

// Initialize things
void CandidTypeNat32::initialize(const uint32_t &v) {
  m_v = v;
  set_datatype();
  encode_I();
  encode_M();
}

// pointer to data in caller, for storing decoded value
void CandidTypeNat32::set_pv(uint32_t *v) { m_pv = v; }

void CandidTypeNat32::set_datatype() {
  m_datatype_opcode = CandidOpcode().Nat32;
  m_datatype_hex = OpcodeHex().Nat32;
  m_datatype_textual = OpcodeTextual().Nat32;
}

void CandidTypeNat32::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
  m_I.append_byte((std::byte)m_datatype_hex);
}

void CandidTypeNat32::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(n : nat<N>)   = i<N>(n)    (Litte Endian)
  m_M.append_int_fixed_width(m_v);
}

// Decode the values, starting at & updating offset
bool CandidTypeNat32::decode_M(VecBytes B, __uint128_t &offset,
                               std::string &parse_error,
                               CandidTypeBase *p_expected) {
  __uint128_t offset_start = offset;
  parse_error = "";
  if (B.parse_int_fixed_width(offset, m_v, parse_error)) {
    std::string to_be_parsed = "Value for CandidTypeNat32";
    CandidDeserialize::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                             parse_error);
  }

  // Fill the user's data placeholder, if a pointer was provided
  if (m_pv) *m_pv = m_v;

  return false;
}