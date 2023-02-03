// The class for the Primitive Candid Type: float64

#include "candid.h"

#include "candid_opcode.h"

CandidTypeFloat64::CandidTypeFloat64() : CandidTypePrim() { initialize(0.0); }

CandidTypeFloat64::CandidTypeFloat64(const double v) : CandidTypePrim() {
  initialize(v);
}

CandidTypeFloat64::~CandidTypeFloat64() {}

// Initialize things
void CandidTypeFloat64::initialize(const double &v) {
  m_v = v;
  set_datatype();
  encode_I();
  encode_M(v);
}

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

void CandidTypeFloat64::encode_M(const double &v) {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(z : float<N>) = f<N>(z)
  m_M.append_float64(v);
}