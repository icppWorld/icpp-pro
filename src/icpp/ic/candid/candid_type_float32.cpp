// The class for the Primitive Candid Type: float32

#include "candid.h"

#include "candid_opcode.h"

CandidTypeFloat32::CandidTypeFloat32() : CandidTypePrim() {
  float v = 0.0;
  initialize(v);
}

// This constructor allows for setting the value during Deserialization
CandidTypeFloat32::CandidTypeFloat32(float *p_v) : CandidTypePrim() {
  set_pv(p_v);

  const float v = const_cast<float &>(*p_v);
  initialize(v);
}

// This constructor is only for encoding
CandidTypeFloat32::CandidTypeFloat32(const float v) : CandidTypePrim() {
  initialize(v);
}

CandidTypeFloat32::~CandidTypeFloat32() {}

// Initialize things
void CandidTypeFloat32::initialize(const float &v) {
  if (sizeof(v) != 4) {
    IC_API::trap(
        "ERROR: The type float is not 4 bytes. \nThis is not yet supported.\n" +
        std::string(__func__));
  }
  m_v = v;
  set_datatype();
  encode_I();
  encode_M();
}

// pointer to data in caller, for storing decoded value
void CandidTypeFloat32::set_pv(float *v) { m_pv = v; }

void CandidTypeFloat32::set_datatype() {
  m_datatype_opcode = CandidOpcode().Float32;
  m_datatype_hex = OpcodeHex().Float32;
  m_datatype_textual = OpcodeTextual().Float32;
}

void CandidTypeFloat32::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
  m_I.append_byte((std::byte)m_datatype_hex);
}

void CandidTypeFloat32::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(z : float<N>) = f<N>(z)
  m_M.append_float_ieee754(m_v);
}

// Decode the values, starting at & updating offset
bool CandidTypeFloat32::decode_M(VecBytes B, __uint128_t &offset,
                                 std::string &parse_error,
                                 CandidTypeBase *p_expected) {
  __uint128_t offset_start = offset;
  parse_error = "";
  if (B.parse_float_ieee754(offset, m_v, parse_error)) {
    std::string to_be_parsed = "Value for CandidTypeFloat32";
    CandidDeserialize::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                             parse_error);
  }

  // Fill the user's data placeholder, if a pointer was provided
  if (m_pv) *m_pv = m_v;

  return false;
}