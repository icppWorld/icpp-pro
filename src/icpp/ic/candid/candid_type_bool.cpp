// The class for the Primitive Candid Type: bool

#include "candid.h"

#include "candid_opcode.h"

CandidTypeBool::CandidTypeBool() : CandidTypePrim() { initialize(true); }

CandidTypeBool::CandidTypeBool(const bool v) : CandidTypePrim() {
  initialize(v);
}

CandidTypeBool::~CandidTypeBool() {}

// Initialize things
void CandidTypeBool::initialize(const bool &v) {
  m_v = v;
  set_datatype();
  encode_I();
  encode_M(v);
}

void CandidTypeBool::set_datatype() {
  m_datatype_opcode = CandidOpcode().Bool;
  m_datatype_hex = OpcodeHex().Bool;
  m_datatype_textual = OpcodeTextual().Bool;
}

void CandidTypeBool::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
  m_I.append_byte((std::byte)m_datatype_hex);
}

void CandidTypeBool::encode_M(const bool &v) {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(b : bool)     = i8(if b then 1 else 0)
  m_M.append_uleb128(v);
}