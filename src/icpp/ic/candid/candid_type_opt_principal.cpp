// The class for the Candid Type: opt

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeOptPrincipal::CandidTypeOptPrincipal() : CandidTypeOptBase() {
  std::optional<std::string> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeOptPrincipal::CandidTypeOptPrincipal(std::optional<std::string> *p_v)
    : CandidTypeOptBase() {
  set_pv(p_v);

  const std::optional<std::string> v =
      const_cast<std::optional<std::string> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeOptPrincipal::CandidTypeOptPrincipal(
    const std::optional<std::string> v)
    : CandidTypeOptBase() {
  set_v(v);
  initialize();
}

CandidTypeOptPrincipal::~CandidTypeOptPrincipal() {}

void CandidTypeOptPrincipal::set_content_type() {
  m_content_type_opcode = CandidOpcode().Principal;
  m_content_type_hex = OpcodeHex().Principal;
  m_content_type_textual = OpcodeTextual().Principal;
}

void CandidTypeOptPrincipal::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(null : opt <datatype>) = i8(0)
  // M(?v   : opt <datatype>) = i8(1) M(v : <datatype>)

  if (!m_v.has_value()) {
    m_M.append_int_fixed_width(uint8_t{0});
  } else {
    m_M.append_int_fixed_width(uint8_t{1});

    std::string s = m_v.value();
    CandidTypePrincipal c{s};
    VecBytes m = c.get_M();
    m_M.append_bytes(m.vec_uint8_t().data(), m.size());
  }
}

// Decode the values, starting at & updating offset
bool CandidTypeOptPrincipal::decode_M(VecBytes B, __uint128_t &offset,
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
    offset_start = offset;
    parse_error = "";
    CandidTypePrincipal c{}; // dummy so we can use it's decode_M
    if (c.decode_M(B, offset, parse_error)) {
      std::string to_be_parsed = "Opt: Value for CandidTypePrincipal";
      CandidDeserialize::trap_with_parse_error(offset_start, offset,
                                               to_be_parsed, parse_error);
    }
    m_v = c.get_v();

  } else if (tag != 0) {
    IC_API::trap("ERROR: tag in opt principal coming from wire is not 0 or 1");
  }

  // Fill the user's data placeholder, if a pointer was provided
  if (m_pv) *m_pv = m_v;

  return false;
}