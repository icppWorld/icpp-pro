// The class for the Primitive Candid Type: float64

#include "candid.h"

#include "candid_opcode.h"

CandidTypeFloat64::CandidTypeFloat64() : CandidTypePrim() {
  double v = 0.0;
  initialize(v);
}

// This constructor allows for setting the value during Deserialization
CandidTypeFloat64::CandidTypeFloat64(double *p_v) : CandidTypePrim() {
  set_pv(p_v);

  const double v = const_cast<double &>(*p_v);
  initialize(v);
}

// This constructor is only for encoding
CandidTypeFloat64::CandidTypeFloat64(const double v) : CandidTypePrim() {
  initialize(v);
}

CandidTypeFloat64::~CandidTypeFloat64() {}

// Initialize things
void CandidTypeFloat64::initialize(const double &v) {
  if (sizeof(v) != 8) {
    IC_API::trap(
        "ERROR: The type double is not 8 bytes. \nThis is not yet supported.\n" +
        std::string(__func__));
  }
  m_v = v;
  set_datatype();
  encode_I();
  encode_M();
}

// pointer to data in caller, for storing decoded value
void CandidTypeFloat64::set_pv(double *v) { m_pv = v; }

void CandidTypeFloat64::set_datatype() {
  m_datatype_opcode = CandidOpcode().Float64;
  m_datatype_hex = OpcodeHex().Float64;
  m_datatype_textual = OpcodeTextual().Float64;
}

void CandidTypeFloat64::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
  m_I.append_byte((std::byte)m_datatype_hex);
}

void CandidTypeFloat64::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(z : float<N>) = f<N>(z)
  m_M.append_float_ieee754(m_v);
}

// Decode the values, starting at & updating offset
bool CandidTypeFloat64::decode_M(VecBytes B, __uint128_t &offset,
                                 std::string &parse_error,
                                 CandidTypeBase *p_expected) {
  __uint128_t offset_start = offset;
  parse_error = "";
  if (B.parse_float_ieee754(offset, m_v, parse_error)) {
    std::string to_be_parsed = "Value for CandidTypeFloat64";
    CandidDeserialize::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                             parse_error);
  }

  // Fill the user's data placeholder, if a pointer was provided
  if (m_pv) *m_pv = m_v;

  return false;
}