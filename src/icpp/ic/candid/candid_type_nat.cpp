// The class for the Primitive Candid Type: nat

#include "candid.h"

#include "candid_opcode.h"

CandidTypeNat::CandidTypeNat() : CandidTypePrim() { initialize(0); }

// This constructor allows for setting the value during Deserialization
CandidTypeNat::CandidTypeNat(__uint128_t *p_v) : CandidTypePrim() {
  set_pv(p_v);

  const __uint128_t v = const_cast<__uint128_t &>(*p_v);
  initialize(v);
}

// This constructor is only for encoding
CandidTypeNat::CandidTypeNat(const __uint128_t v) : CandidTypePrim() {
  initialize(v);
}

CandidTypeNat::~CandidTypeNat() {}

// Initialize things
void CandidTypeNat::initialize(const __uint128_t &v) {
  m_v = v;
  set_datatype();
  encode_I();
  encode_M();
}

// pointer to data in caller, for storing decoded value
void CandidTypeNat::set_pv(__uint128_t *v) { m_pv = v; }

void CandidTypeNat::set_datatype() {
  m_datatype_opcode = CandidOpcode().Nat;
  m_datatype_hex = OpcodeHex().Nat;
  m_datatype_textual = OpcodeTextual().Nat;
}

void CandidTypeNat::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
  m_I.append_byte((std::byte)m_datatype_hex);
}

void CandidTypeNat::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(n : nat)      = leb128(n)
  m_M.append_uleb128(m_v);
}

// Decode the values, starting at & updating offset
bool CandidTypeNat::decode_M(VecBytes B, __uint128_t &offset,
                             std::string &parse_error,
                             CandidTypeBase *p_expected) {
  __uint128_t offset_start = offset;
  __uint128_t numbytes;
  parse_error = "";
  if (B.parse_uleb128(offset, m_v, numbytes, parse_error)) {
    std::string to_be_parsed = "Value for CandidTypeNat";
    CandidDeserialize::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                             parse_error);
  }

  // Fill the user's data placeholder, if a pointer was provided
  if (m_pv) *m_pv = m_v;

  return false;
}