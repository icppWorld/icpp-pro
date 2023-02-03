// The class for the Primitive Candid Type: nat

#include "candid.h"

#include "candid_opcode.h"

CandidTypeNat::CandidTypeNat() : CandidTypePrim() { initialize(0); }

CandidTypeNat::CandidTypeNat(const __uint128_t v) : CandidTypePrim() {
  initialize(v);
}

CandidTypeNat::~CandidTypeNat() {}

// Initialize things
void CandidTypeNat::initialize(const __uint128_t &v) {
  m_v = v;
  set_datatype();
  encode_I();
  encode_M(v);
}

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

void CandidTypeNat::encode_M(const __uint128_t &v) {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(n : nat)      = leb128(n)
  m_M.append_uleb128(v);
}