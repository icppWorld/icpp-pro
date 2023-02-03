// The class for the Primitive Candid Type: empty

#include "candid.h"

#include "candid_opcode.h"

CandidTypeEmpty::CandidTypeEmpty() : CandidTypePrim() {
  set_datatype();
  encode_I();
  encode_M();
}

CandidTypeEmpty::~CandidTypeEmpty() {}

void CandidTypeEmpty::set_datatype() {
  m_datatype_opcode = CandidOpcode().Empty;
  m_datatype_hex = OpcodeHex().Empty;
  m_datatype_textual = OpcodeTextual().Empty;
}

void CandidTypeEmpty::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
  m_I.append_byte((std::byte)m_datatype_hex);
}

void CandidTypeEmpty::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // NB: M(_ : empty) will never be called
  m_M.clear();
}