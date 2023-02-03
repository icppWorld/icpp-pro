// Candid type table
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results

#include "candid.h"
#include "candid_opcode.h"
#include "ic_api.h"

#include <cassert>
#include <utility>

#include "ZipIterator.hpp"

CandidTypeTable::CandidTypeTable() {}

CandidTypeTable::CandidTypeTable(const VecBytes &B, __uint128_t &B_offset) {
  m_B = B;
  m_B_offset = B_offset;
  deserialize(B_offset);
}

CandidTypeTable::~CandidTypeTable() {}

void CandidTypeTable::deserialize(__uint128_t &B_offset) {
  // Get the datatype for the type table
  __uint128_t B_offset_start = B_offset;
  std::string parse_error;
  __int128_t datatype;
  __uint128_t numbytes;
  if (m_B.parse_sleb128(B_offset, datatype, numbytes, parse_error)) {
    std::string to_be_parsed = "Type table: opcode";
    CandidDeserialize::trap_with_parse_error(B_offset_start, B_offset,
                                             to_be_parsed, parse_error);
  }
  m_opcode = int(datatype);

  // Deserialize the type-table using the decoder of the datatype
  CandidOpcode().candid_type_from_opcode(m_c, m_opcode);
  parse_error = "";
  if (std::visit(
          [&](auto &&c) { return c.decode_T(m_B, B_offset, parse_error); },
          m_c)) {
    std::string to_be_parsed = "Type table";
    CandidDeserialize::trap_with_parse_error(B_offset_start, B_offset,
                                             to_be_parsed, parse_error);
  }
}