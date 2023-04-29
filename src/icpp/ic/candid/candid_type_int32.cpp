// The class for the Primitive Candid Type: int32

#include "candid.h"

#include "candid_opcode.h"
#include "pro.h"

CandidTypeInt32::CandidTypeInt32() : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeInt32::CandidTypeInt32(int32_t *p_v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeInt32::CandidTypeInt32(const int32_t v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
  ;
}

CandidTypeInt32::~CandidTypeInt32() {}

void CandidTypeInt32::initialize(const int32_t &v) {}

void CandidTypeInt32::set_pv(int32_t *v) {}

void CandidTypeInt32::set_datatype() {}

void CandidTypeInt32::encode_I() {}

void CandidTypeInt32::encode_M() {}

bool CandidTypeInt32::decode_M(VecBytes B, __uint128_t &offset,
                               std::string &parse_error) {

  return false;
}