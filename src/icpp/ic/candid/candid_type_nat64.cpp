// The class for the Primitive Candid Type: nat64

#include "candid.h"

#include "candid_opcode.h"

CandidTypeNat64::CandidTypeNat64() : CandidTypePrim() { initialize(0); }

// This constructor allows for setting the value during Deserialization
CandidTypeNat64::CandidTypeNat64(uint64_t *p_v) : CandidTypePrim() {
  set_pv(p_v);

  const uint64_t v = const_cast<uint64_t &>(*p_v);
  initialize(v);
}

// This constructor is only for encoding
CandidTypeNat64::CandidTypeNat64(const uint64_t v) : CandidTypePrim() {
  initialize(v);
}

CandidTypeNat64::~CandidTypeNat64() {}

// Initialize things
void CandidTypeNat64::initialize(const uint64_t &v) {
  m_v = v;
  set_datatype();
  encode_I();
  encode_M();
}

// pointer to data in caller, for storing decoded value
void CandidTypeNat64::set_pv(uint64_t *v) { m_pv = v; }

void CandidTypeNat64::set_datatype() {
  m_datatype_opcode = CandidOpcode().Nat64;
  m_datatype_hex = OpcodeHex().Nat64;
  m_datatype_textual = OpcodeTextual().Nat64;
}

void CandidTypeNat64::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
  m_I.append_byte((std::byte)m_datatype_hex);
}

void CandidTypeNat64::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(n : nat<N>)   = i<N>(n)    (Litte Endian)
  m_M.append_int_fixed_width(m_v);
}

// Decode the values, starting at & updating offset
bool CandidTypeNat64::decode_M(VecBytes B, __uint128_t &offset,
                               std::string &parse_error,
                               CandidTypeBase *p_expected) {
  __uint128_t offset_start = offset;
  parse_error = "";
  if (B.parse_int_fixed_width(offset, m_v, parse_error)) {
    std::string to_be_parsed = "Value for CandidTypeNat64";
    CandidDeserialize::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                             parse_error);
  }

  // Fill the user's data placeholder, if a pointer was provided
  if (m_pv) *m_pv = m_v;

  return false;
}