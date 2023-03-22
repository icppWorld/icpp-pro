// The class for the Primitive Candid Type: text

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "cppcodec/base32_rfc4648.hpp"
#include "hash-library/crc32.h"

#include "ic_api.h"

//                                                               data_bytes
const std::string PRINCIPAL_ID_ANONYMOUS = "2vxsx-fae";          // 1 byte: x04
const std::string PRINCIPAL_ID_MANAGEMENT_CANISTER = "aaaaa-aa"; // no bytes

const uint32_t MAX_LENGTH_IN_BYTES = 29;
const uint32_t CRC_LENGTH_IN_BYTES = 4;

const std::string PRINCIPAL_OPAQUE_REFERENCE_NOT_SUPPORTED =
    "Principal Error: Found a value different that 1 for the reference. Opaque reference not supported.";
const std::string PRINCIPAL_ERROR_BYTES_TOO_LONG =
    "Principal Error: Bytes is longer than 29 bytes.";
const std::string PRINCIPAL_ERROR_INVALID_BASE32 =
    "Principal Error: Text must be in valid Base32 encoding.";
const std::string PRINCIPAL_ERROR_TEXT_TOO_SHORT =
    "Principal Error: Text is too short.";
const std::string PRINCIPAL_ERROR_TEXT_TOO_LONG =
    "Principal Error: Text is too long.";
const std::string PRINCIPAL_ERROR_CHECK_SEQUENCE_NOT_MATCH =
    "Principal Error: CRC32 checksum doesn't match.";
const std::string PRINCIPAL_ERROR_ABNORMAL_GROUPED =
    "Principal Error: Text should be separated by - (dash) every 5 characters";
const std::string PRINCIPAL_ERROR_INVALID_BASE32_ENCODING =
    "Principal Error: Text must be in valid Base32 encoding. Decoding/Encoding roundtrip fails.";

CandidTypePrincipal::CandidTypePrincipal() : CandidTypePrim() {
  initialize("");
}

// These constructors allows for setting the value during Deserialization
CandidTypePrincipal::CandidTypePrincipal(std::string *p_v) : CandidTypePrim() {
  set_pv(p_v);

  const std::string v = const_cast<std::string &>(*p_v);
  initialize(v);
}

// These constructors are only for encoding
CandidTypePrincipal::CandidTypePrincipal(const char *c) : CandidTypePrim() {
  std::string v(c);
  initialize(v);
}
CandidTypePrincipal::CandidTypePrincipal(const std::string v)
    : CandidTypePrim() {
  initialize(v);
}

CandidTypePrincipal::~CandidTypePrincipal() {}

// Initialize things
void CandidTypePrincipal::initialize(const std::string &v) {
  if (v == "") {
    m_v = PRINCIPAL_ID_ANONYMOUS;
    // Fill the user's data placeholder, if a pointer was provided
    if (m_pv) *m_pv = m_v;
  } else m_v = v;
  bytes_from_string();
  set_datatype();
  encode_I();
  encode_M();
}

// pointer to data in caller, for storing decoded value
void CandidTypePrincipal::set_pv(std::string *v) { m_pv = v; }

void CandidTypePrincipal::set_datatype() {
  m_datatype_opcode = CandidOpcode().Principal;
  m_datatype_hex = OpcodeHex().Principal;
  m_datatype_textual = OpcodeTextual().Principal;
}

void CandidTypePrincipal::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
  m_I.append_byte((std::byte)m_datatype_hex);
}

void CandidTypePrincipal::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(ref(r) : principal) = i8(0)                      <= Unused opaque reference (https://forum.dfinity.org/t/candid-question-regarding-ref-r/18943/2?u=icpp)
  // M(id(v*) : principal) = i8(1) M(v* : vec nat8)     <= length + bytes

  // Unused opaque reference: i8(0)
  // Just append the value 1. This is the convention
  uint8_t ref_r = 1;
  m_M.append_int_fixed_width(ref_r);

  // encoded size - i8(1)
  m_M.append_uleb128(__uint128_t(m_v_bytes.size()));

  // encoded data - M(v* : vec nat8)
  m_M.append_bytes((const uint8_t *)(void *)m_v_bytes.vec().data(),
                   m_v_bytes.vec().size());
}

// Decode the values, starting at & updating offset
bool CandidTypePrincipal::decode_M(VecBytes B, __uint128_t &offset,
                                   std::string &parse_error,
                                   CandidTypeBase *p_expected) {

  // Get the unused opaque reference: i8(0)
  // Verify that the value is 1. This is the convention
  __uint128_t offset_start = offset;
  parse_error = "";
  uint8_t ref_r;
  if (B.parse_int_fixed_width(offset, ref_r, parse_error)) {
    std::string to_be_parsed = "Unused opaque reference.";
    CandidDeserialize::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                             parse_error);
  }
  if (ref_r != 1) {
    IC_API::trap(PRINCIPAL_OPAQUE_REFERENCE_NOT_SUPPORTED);
  }

  // Get the length
  __uint128_t numbytes;
  parse_error = "";
  __uint128_t numBytes_principal;
  if (B.parse_uleb128(offset, numBytes_principal, numbytes, parse_error)) {
    std::string to_be_parsed = "Size of data bytes for principal";
    CandidDeserialize::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                             parse_error);
  }

  // Get the encoded principal bytes as uint8_t
  std::vector<uint8_t> data_bytes;
  if (B.parse_bytes(offset, data_bytes, numBytes_principal, numbytes,
                    parse_error)) {
    std::string to_be_parsed = "Data bytes for principal";
    CandidDeserialize::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                             parse_error);
  }

  m_v = string_from_bytes(data_bytes);

  // Fill the user's data placeholder, if a pointer was provided
  if (m_pv) *m_pv = m_v;

  return false;
}

// convert string representation of principal into bytes
void CandidTypePrincipal::bytes_from_string() {
  // spec: https://internetcomputer.org/docs/current/references/id-encoding-spec
  // Do the reverse of: m_v = Encode(data) := Group(LowerCase(Base32(CRC32(data) || data)))

  std::string s = m_v;
  ungroup(s);
  make_ascii_uppercase(s);
  std::vector<uint8_t> bytes = base32_decode(s);

  if (bytes.size() < CRC_LENGTH_IN_BYTES) {
    IC_API::trap(PRINCIPAL_ERROR_TEXT_TOO_SHORT);
  }

  // First 4 bytes are a CRC32 checksum of the data that follows. The data bytes are the principal ID
  std::vector<uint8_t> crc_bytes(bytes.begin(),
                                 bytes.begin() + CRC_LENGTH_IN_BYTES);
  std::vector<uint8_t> data_bytes(bytes.begin() + CRC_LENGTH_IN_BYTES,
                                  bytes.end());

  if (data_bytes.size() > MAX_LENGTH_IN_BYTES) {
    IC_API::trap(PRINCIPAL_ERROR_TEXT_TOO_LONG);
  }

  // Verify the CRC32 checksum of the data bytes
  std::array<uint8_t, 4> crc_data = crc32(data_bytes);
  if (!std::equal(crc_bytes.begin(), crc_bytes.end(), crc_data.begin())) {
    IC_API::trap(PRINCIPAL_ERROR_CHECK_SEQUENCE_NOT_MATCH);
  }

  // from https://internetcomputer.org/docs/current/references/id-encoding-spec
  //   w3gef-eqbai -> 0102 -> w3gef-eqbai
  //   w3gef-eqbaj -> 0102 -> w3gef-eqbai ==> Error due to non-zero padding bits
  //
  // Verify that roundtrip back to string gives same result
  std::string s_roundtrip = string_from_bytes(data_bytes);
  if (s_roundtrip != m_v) {
    IC_API::trap(PRINCIPAL_ERROR_INVALID_BASE32_ENCODING);
  }

  // All is OK, store the bytes of the Principal ID
  m_v_bytes.append_bytes(data_bytes.data(), data_bytes.size());
}

// convert bytes of principal into string representation
std::string
CandidTypePrincipal::string_from_bytes(std::vector<uint8_t> &data_bytes) {
  // spec: https://internetcomputer.org/docs/current/references/id-encoding-spec
  // m_v = Encode(data) := Group(LowerCase(Base32(CRC32(data) || data)))

  if (data_bytes.size() > MAX_LENGTH_IN_BYTES) {
    IC_API::trap(PRINCIPAL_ERROR_TEXT_TOO_LONG);
  }

  // CRC32(data) - checksum of the data bytes
  std::array<uint8_t, 4> crc_bytes = crc32(data_bytes);

  // Base32(CRC32(data) || data)
  std::vector<uint8_t> bytes(crc_bytes.begin(), crc_bytes.end());
  bytes.insert(bytes.end(), data_bytes.begin(), data_bytes.end());
  std::string s = base32_encode(bytes);

  // Group(LowerCase(Base32(CRC32(data) || data)))
  make_ascii_lowercase(s);
  group(s);

  return s;
}

void CandidTypePrincipal::ungroup(std::string &s) {
  for (size_t i = 5; i < s.size(); i += 6) {
    if (s[i] != '-') {
      IC_API::trap(PRINCIPAL_ERROR_ABNORMAL_GROUPED);
    }
  }
  s.erase(std::remove(s.begin(), s.end(), '-'), s.end());
}

void CandidTypePrincipal::group(std::string &s) {
  if (s.size() > 0) {
    std::string ss = s;
    s.clear();
    s.push_back(ss[0]);
    for (size_t i = 1; i < ss.size(); ++i) {
      if (i % 5 == 0) s.push_back('-');
      s.push_back(ss[i]);
    }
  }
}

std::vector<uint8_t> CandidTypePrincipal::base32_decode(std::string &s) {
  // cppcodec requires padding with "="
  pad(s);
  return cppcodec::base32_rfc4648::decode(s);
}

std::string CandidTypePrincipal::base32_encode(std::vector<uint8_t> &bytes) {
  std::string s = cppcodec::base32_rfc4648::encode(bytes);
  nopad(s);
  return s;
}

void CandidTypePrincipal::pad(std::string &s) {
  uint64_t l = s.size();
  uint64_t m = l % 8;

  if (m > 0) {
    uint64_t pad_len = 8 - m;
    s.append(pad_len, '=');
    assert(s.size() % 8 == 0);
  }
}

void CandidTypePrincipal::nopad(std::string &s) {
  s.erase(std::remove(s.begin(), s.end(), '='), s.end());
}

std::array<uint8_t, 4>
CandidTypePrincipal::crc32(std::vector<uint8_t> data_bytes) {
  CRC32 crc32;
  std::string crc32_data_str = crc32(data_bytes.data(), data_bytes.size());
  std::array<uint8_t, 4> crc_data;
  crc32.getHash(crc_data.data());
  return crc_data;
}