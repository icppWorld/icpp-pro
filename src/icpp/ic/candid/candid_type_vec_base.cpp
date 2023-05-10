// The base class for the Candid Type: vec

#include "candid.h"
#include "candid_opcode.h"
#include "pro.h"

#include <cassert>

#include "ic_api.h"

CandidTypeVecBase::CandidTypeVecBase() : CandidTypeBase() {}

CandidTypeVecBase::~CandidTypeVecBase() {}

// Initialize things
void CandidTypeVecBase::initialize() {
  set_datatype();
  set_content_type();
  encode_T();
  encode_I();
  encode_M();
}

void CandidTypeVecBase::set_datatype() {
  m_datatype_opcode = CandidOpcode().Vec;
  m_datatype_hex = OpcodeHex().Vec;
  m_datatype_textual = OpcodeTextual().Vec;
}

// build the type table encoding
void CandidTypeVecBase::encode_T() {
  m_T.append_byte((std::byte)m_datatype_hex);
  m_T.append_byte((std::byte)m_content_type_hex);
}

// Decode the type table, starting at & updating offset
bool CandidTypeVecBase::decode_T(VecBytes B, __uint128_t &offset,
                                 std::string &parse_error) {
  __uint128_t len = B.size() - offset;

  // The opcode for content type
  __uint128_t offset_start = offset;
  parse_error = "";
  __int128_t content_type;
  __uint128_t numbytes;
  if (B.parse_sleb128(offset, content_type, numbytes, parse_error)) {
    std::string to_be_parsed = "Type table: a Vec's content type";
    CandidDeserialize::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                             parse_error);
  }

  m_content_type_opcode = int(content_type);
  return false;
}

// For vecs, we set the Opcode, but note that it is not used during serialization.
// At serialization time, we use the index in the overall type table.
//
// Encode the datatype
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// For <constype>: the negative Opcode
void CandidTypeVecBase::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}
