// The class for the Primitive Candid Type: reserved

#include "candid.h"
#include "ic_api.h"

#include "candid_opcode.h"

CandidTypeReserved::CandidTypeReserved() : CandidTypePrim() {
  set_datatype();
  encode_I();
  encode_M();
}

CandidTypeReserved::~CandidTypeReserved() {}

void CandidTypeReserved::set_datatype() {
  m_datatype_opcode = CandidOpcode().Reserved;
  m_datatype_hex = OpcodeHex().Reserved;
  m_datatype_textual = OpcodeTextual().Reserved;
}

void CandidTypeReserved::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
  m_I.append_byte((std::byte)m_datatype_hex);
}

void CandidTypeReserved::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // To be ignored
  m_M.clear();
}

// Decode the values, starting at & updating offset
bool CandidTypeReserved::decode_M(VecBytes B, __uint128_t &offset,
                                  std::string &parse_error,
                                  CandidTypeBase *p_expected) {
  // To be ignored
  return false;
}