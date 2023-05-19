// The class for the Candid Type: opt

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeOptInt64::CandidTypeOptInt64() : CandidTypeOptBase() {
  std::optional<int64_t> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeOptInt64::CandidTypeOptInt64(std::optional<int64_t> *p_v)
    : CandidTypeOptBase() {
  set_pv(p_v);

  const std::optional<int64_t> v = const_cast<std::optional<int64_t> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeOptInt64::CandidTypeOptInt64(const std::optional<int64_t> v)
    : CandidTypeOptBase() {
  set_v(v);
  initialize();
}

CandidTypeOptInt64::~CandidTypeOptInt64() {}

void CandidTypeOptInt64::set_content_type() {
  m_content_type_opcode = CandidOpcode().Int64;
  m_content_type_hex = OpcodeHex().Int64;
  m_content_type_textual = OpcodeTextual().Int64;
}

void CandidTypeOptInt64::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(null : opt <datatype>) = i8(0)
  // M(?v   : opt <datatype>) = i8(1) M(v : <datatype>)

  if (!m_v.has_value()) {
    m_M.append_int_fixed_width(uint8_t{0});
  } else {
    m_M.append_int_fixed_width(uint8_t{1});
    m_M.append_int_fixed_width(m_v.value());
  }
}

// Decode the values, starting at & updating offset
bool CandidTypeOptInt64::decode_M(VecBytes B, __uint128_t &offset,
                                  std::string &parse_error) {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(null : opt <datatype>) = i8(0)
  // M(?v   : opt <datatype>) = i8(1) M(v : <datatype>)

  // get tag of opt - i8(0)
  // this will also work if there is a null on the wire.
  __uint128_t offset_start = offset;
  parse_error = "";
  uint8_t tag;
  if (B.parse_int_fixed_width(offset, tag, parse_error)) {
    std::string to_be_parsed = "Opt tag.";
    CandidDeserialize::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                             parse_error);
  }
  if (tag == 1) {
    int64_t v;
    offset_start = offset;
    parse_error = "";
    if (B.parse_int_fixed_width(offset, v, parse_error)) {
      std::string to_be_parsed = "Opt: Value for CandidTypeInt64";
      CandidDeserialize::trap_with_parse_error(offset_start, offset,
                                               to_be_parsed, parse_error);
    }
    m_v = v;

  } else if (tag != 0) {
    IC_API::trap("ERROR: tag in opt int64 coming from wire is not 0 or 1");
  }

  // Fill the user's data placeholder, if a pointer was provided
  if (m_pv) *m_pv = m_v;

  return false;
}