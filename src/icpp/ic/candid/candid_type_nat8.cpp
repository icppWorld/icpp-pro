// The class for the Primitive Candid Type: nat8

#include "candid.h"

#include "candid_opcode.h"
#include "pro.h"

CandidTypeNat8::CandidTypeNat8() : CandidTypePrim() { Pro().exit_if_not_pro(); }

CandidTypeNat8::CandidTypeNat8(uint8_t *p_v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeNat8::CandidTypeNat8(const uint8_t v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeNat8::~CandidTypeNat8() {}

void CandidTypeNat8::initialize(const uint8_t &v) {}

void CandidTypeNat8::set_pv(uint8_t *v) {}

void CandidTypeNat8::set_datatype() {}

void CandidTypeNat8::encode_I() {}

void CandidTypeNat8::encode_M() {}

bool CandidTypeNat8::decode_M(VecBytes B, __uint128_t &offset,
                              std::string &parse_error) {
  return false;
}