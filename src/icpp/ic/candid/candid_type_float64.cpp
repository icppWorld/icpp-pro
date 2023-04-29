// The class for the Primitive Candid Type: float64

#include "candid.h"

#include "candid_opcode.h"
#include "pro.h"

CandidTypeFloat64::CandidTypeFloat64() : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeFloat64::CandidTypeFloat64(double *p_v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeFloat64::CandidTypeFloat64(const double v) : CandidTypePrim() {
  Pro().exit_if_not_pro();
}

CandidTypeFloat64::~CandidTypeFloat64() {}

void CandidTypeFloat64::initialize(const double &v) {}

void CandidTypeFloat64::set_pv(double *v) {}

void CandidTypeFloat64::set_datatype() {}

void CandidTypeFloat64::encode_I() {}

void CandidTypeFloat64::encode_M() {}

bool CandidTypeFloat64::decode_M(VecBytes B, __uint128_t &offset,
                                 std::string &parse_error) {
  return false;
}