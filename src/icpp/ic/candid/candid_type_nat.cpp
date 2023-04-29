// The class for the Primitive Candid Type: nat

#include "candid.h"

#include "candid_opcode.h"
#include "pro.h"

CandidTypeNat::CandidTypeNat() : CandidTypePrim() { Pro().exit_if_not_pro(); }

CandidTypeNat::CandidTypeNat(__uint128_t *p_v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeNat::CandidTypeNat(const __uint128_t v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeNat::~CandidTypeNat() {}

void CandidTypeNat::initialize(const __uint128_t &v) {}

void CandidTypeNat::set_pv(__uint128_t *v) { m_pv = v; }

void CandidTypeNat::set_datatype() {}

void CandidTypeNat::encode_I() {}

void CandidTypeNat::encode_M() {}

bool CandidTypeNat::decode_M(VecBytes B, __uint128_t &offset,
                             std::string &parse_error) {
  return false;
}