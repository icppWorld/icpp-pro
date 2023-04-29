// The class for the Candid Type: vec

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeVecInt::CandidTypeVecInt() : CandidTypeVecBase() {}

CandidTypeVecInt::CandidTypeVecInt(std::vector<__int128_t> *p_v)
    : CandidTypeVecBase() {}

CandidTypeVecInt::CandidTypeVecInt(const std::vector<__int128_t> v)
    : CandidTypeVecBase() {}

CandidTypeVecInt::~CandidTypeVecInt() {}

void CandidTypeVecInt::set_content_type() {}

void CandidTypeVecInt::encode_M() {}

bool CandidTypeVecInt::decode_M(VecBytes B, __uint128_t &offset,
                                std::string &parse_error) {
  return false;
}