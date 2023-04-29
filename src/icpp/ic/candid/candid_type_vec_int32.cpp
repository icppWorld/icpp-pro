// The class for the Candid Type: vec

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeVecInt32::CandidTypeVecInt32() : CandidTypeVecBase() {}

CandidTypeVecInt32::CandidTypeVecInt32(std::vector<int32_t> *p_v)
    : CandidTypeVecBase() {}

CandidTypeVecInt32::CandidTypeVecInt32(const std::vector<int32_t> v)
    : CandidTypeVecBase() {}

CandidTypeVecInt32::~CandidTypeVecInt32() {}

void CandidTypeVecInt32::set_content_type() {}

void CandidTypeVecInt32::encode_M() {}

bool CandidTypeVecInt32::decode_M(VecBytes B, __uint128_t &offset,
                                  std::string &parse_error) {
  return false;
}