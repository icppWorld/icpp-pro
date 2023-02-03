// The class for the Primitive Candid Type: text

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeText::CandidTypeText() : CandidTypePrim() { initialize(""); }

// These constructors allows for setting the value during Deserialization
CandidTypeText::CandidTypeText(std::string *p_v) : CandidTypePrim() {
  set_pv(p_v);

  const std::string v = const_cast<std::string &>(*p_v);
  initialize(v);
}

// These constructors are only for encoding
CandidTypeText::CandidTypeText(const char *c) : CandidTypePrim() {
  std::string v(c);
  initialize(v);
}
CandidTypeText::CandidTypeText(const std::string v) : CandidTypePrim() {
  initialize(v);
}

CandidTypeText::~CandidTypeText() {}

// Initialize things
void CandidTypeText::initialize(const std::string &v) {
  m_v = v;
  set_datatype();
  encode_I();
  encode_M(v);
}

// pointer to data in caller, for storing decoded value
void CandidTypeText::set_pv(std::string *v) { m_pv = v; }

void CandidTypeText::set_datatype() {
  m_datatype_opcode = CandidOpcode().Text;
  m_datatype_hex = OpcodeHex().Text;
  m_datatype_textual = OpcodeTextual().Text;
}

void CandidTypeText::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
  m_I.append_byte((std::byte)m_datatype_hex);
}

void CandidTypeText::encode_M(const std::string &v) {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(t : text)     = leb128(|utf8(t)|) i8*(utf8(t))

  // encoded size of string - leb128(|utf8(t)|)
  m_M.append_uint64_t(v.size());

  // encoded string - i8*(utf8(t))
  for (char const &c : v) {
    m_M.append_byte((std::byte)c);
  }
}

// Decode the values, starting at & updating offset
bool CandidTypeText::decode_M(VecBytes B, __uint128_t &offset,
                              std::string &parse_error,
                              CandidTypeBase *p_expected) {

  // get size of text - leb128(|utf8(t)|)
  __uint128_t offset_start = offset;
  __uint128_t numbytes;
  parse_error = "";
  uint64_t numBytes_text;
  if (B.parse_uleb128(offset, numBytes_text, numbytes, parse_error)) {
    std::string to_be_parsed = "Size of text- leb128(|utf8(t)|)";
    CandidDeserialize::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                             parse_error);
  }

  // Get the text
  int len = B.size() - offset;

  std::vector<std::byte> vec = B.vec();

  const std::byte *data = vec.data();
  if (numBytes_text > len) {
    std::string msg =
        "ERROR - Size of text is wrong. The remaining bytes in the byte stream on wire is ";
    msg.append(std::to_string(len));
    msg.append(", but specified size is ");
    msg.append(std::to_string(numBytes_text));
    IC_API::trap(msg);
  }

  m_v = "";
  for (size_t i = 0; i < numBytes_text; ++i) {
    m_v.append(VecBytes::byte_to_char(vec[offset]));

    ++offset;
    --len;
  }

  // Fill the user's data placeholder, if a pointer was provided
  if (m_pv) *m_pv = m_v;

  return false;
}