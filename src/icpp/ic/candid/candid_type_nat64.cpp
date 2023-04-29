// The class for the Primitive Candid Type: nat64

#include "candid.h"

#include "candid_opcode.h"
#include "pro.h"

CandidTypeNat64::CandidTypeNat64() : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeNat64::CandidTypeNat64(uint64_t *p_v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeNat64::CandidTypeNat64(const uint64_t v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeNat64::~CandidTypeNat64() {}

void CandidTypeNat64::initialize(const uint64_t &v) {}

void CandidTypeNat64::set_pv(uint64_t *v) {}

void CandidTypeNat64::set_datatype() {}

void CandidTypeNat64::encode_I() {}

void CandidTypeNat64::encode_M() {}

bool CandidTypeNat64::decode_M(VecBytes B, __uint128_t &offset,
                               std::string &parse_error) {
  return false;
}