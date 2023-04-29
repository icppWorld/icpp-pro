// The class for the Primitive Candid Type: nat16

#include "candid.h"

#include "candid_opcode.h"
#include "pro.h"

CandidTypeNat16::CandidTypeNat16() : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeNat16::CandidTypeNat16(uint16_t *p_v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeNat16::CandidTypeNat16(const uint16_t v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeNat16::~CandidTypeNat16() {}

void CandidTypeNat16::initialize(const uint16_t &v) {}

void CandidTypeNat16::set_pv(uint16_t *v) { m_pv = v; }

void CandidTypeNat16::set_datatype() {}

void CandidTypeNat16::encode_I() {}

void CandidTypeNat16::encode_M() {}

bool CandidTypeNat16::decode_M(VecBytes B, __uint128_t &offset,
                               std::string &parse_error) {
  return false;
}