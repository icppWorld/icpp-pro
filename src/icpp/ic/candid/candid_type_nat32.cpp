// The class for the Primitive Candid Type: nat32

#include "candid.h"

#include "candid_opcode.h"
#include "pro.h"

CandidTypeNat32::CandidTypeNat32() : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeNat32::CandidTypeNat32(uint32_t *p_v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeNat32::CandidTypeNat32(const uint32_t v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeNat32::~CandidTypeNat32() {}

void CandidTypeNat32::initialize(const uint32_t &v) {}

void CandidTypeNat32::set_pv(uint32_t *v) { m_pv = v; }

void CandidTypeNat32::set_datatype() {}

void CandidTypeNat32::encode_I() {}

void CandidTypeNat32::encode_M() {}

bool CandidTypeNat32::decode_M(VecBytes B, __uint128_t &offset,
                               std::string &parse_error) {
  return false;
}