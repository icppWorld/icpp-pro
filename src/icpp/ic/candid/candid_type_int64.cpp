// The class for the Primitive Candid Type: int64

#include "candid.h"

#include "candid_opcode.h"
#include "pro.h"

CandidTypeInt64::CandidTypeInt64() : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeInt64::CandidTypeInt64(int64_t *p_v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeInt64::CandidTypeInt64(const int64_t v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeInt64::~CandidTypeInt64() {}

void CandidTypeInt64::initialize(const int64_t &v) {}

void CandidTypeInt64::set_pv(int64_t *v) {}

void CandidTypeInt64::set_datatype() {}

void CandidTypeInt64::encode_I() {}

void CandidTypeInt64::encode_M() {}

bool CandidTypeInt64::decode_M(VecBytes B, __uint128_t &offset,
                               std::string &parse_error) {
  return false;
}