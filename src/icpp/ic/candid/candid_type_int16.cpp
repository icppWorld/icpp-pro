// The class for the Primitive Candid Type: int16

#include "candid.h"

#include "candid_opcode.h"
#include "pro.h"

CandidTypeInt16::CandidTypeInt16() : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeInt16::CandidTypeInt16(int16_t *p_v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeInt16::CandidTypeInt16(const int16_t v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeInt16::~CandidTypeInt16() {}

void CandidTypeInt16::initialize(const int16_t &v) {}

void CandidTypeInt16::set_pv(int16_t *v) {}

void CandidTypeInt16::set_datatype() {}

void CandidTypeInt16::encode_I() {}

void CandidTypeInt16::encode_M() {}

bool CandidTypeInt16::decode_M(VecBytes B, __uint128_t &offset,
                               std::string &parse_error) {
  return false;
}