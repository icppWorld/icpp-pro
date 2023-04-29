// The class for the Candid Type: vec

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeVecInt64::CandidTypeVecInt64() : CandidTypeVecBase() {}

CandidTypeVecInt64::CandidTypeVecInt64(std::vector<int64_t> *p_v)
    : CandidTypeVecBase() {}

CandidTypeVecInt64::CandidTypeVecInt64(const std::vector<int64_t> v)
    : CandidTypeVecBase() {}

CandidTypeVecInt64::~CandidTypeVecInt64() {}

void CandidTypeVecInt64::set_content_type() {}

void CandidTypeVecInt64::encode_M() {}

bool CandidTypeVecInt64::decode_M(VecBytes B, __uint128_t &offset,
                                  std::string &parse_error) {
  return false;
}