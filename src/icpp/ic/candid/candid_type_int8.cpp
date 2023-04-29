// The class for the Primitive Candid Type: int8

#include "candid.h"

#include "candid_opcode.h"
#include "pro.h"

CandidTypeInt8::CandidTypeInt8() : CandidTypePrim() { Pro().exit_if_not_pro(); }

CandidTypeInt8::CandidTypeInt8(int8_t *p_v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeInt8::CandidTypeInt8(const int8_t v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeInt8::~CandidTypeInt8() {}

void CandidTypeInt8::initialize(const int8_t &v) {}

void CandidTypeInt8::set_pv(int8_t *v) {}

void CandidTypeInt8::set_datatype() {}

void CandidTypeInt8::encode_I() {}

void CandidTypeInt8::encode_M() {}

bool CandidTypeInt8::decode_M(VecBytes B, __uint128_t &offset,
                              std::string &parse_error) {

  return false;
}