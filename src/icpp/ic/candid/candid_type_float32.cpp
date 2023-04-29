// The class for the Primitive Candid Type: float32

#include "candid.h"

#include "candid_opcode.h"
#include "pro.h"

CandidTypeFloat32::CandidTypeFloat32() : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeFloat32::CandidTypeFloat32(float *p_v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeFloat32::CandidTypeFloat32(const float v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeFloat32::~CandidTypeFloat32() {}

void CandidTypeFloat32::initialize(const float &v) {}

void CandidTypeFloat32::set_pv(float *v) {}

void CandidTypeFloat32::set_datatype() {}

void CandidTypeFloat32::encode_I() {}

void CandidTypeFloat32::encode_M() {}

bool CandidTypeFloat32::decode_M(VecBytes B, __uint128_t &offset,
                                 std::string &parse_error) {
  return false;
}