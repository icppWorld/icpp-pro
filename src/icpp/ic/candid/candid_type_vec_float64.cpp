// The class for the Candid Type: vec

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeVecFloat64::CandidTypeVecFloat64() : CandidTypeVecBase() {}

CandidTypeVecFloat64::CandidTypeVecFloat64(std::vector<double> *p_v)
    : CandidTypeVecBase() {}

CandidTypeVecFloat64::CandidTypeVecFloat64(const std::vector<double> v)
    : CandidTypeVecBase() {}

CandidTypeVecFloat64::~CandidTypeVecFloat64() {}

void CandidTypeVecFloat64::set_content_type() {}

void CandidTypeVecFloat64::encode_M() {}

bool CandidTypeVecFloat64::decode_M(VecBytes B, __uint128_t &offset,
                                    std::string &parse_error) {
  return false;
}