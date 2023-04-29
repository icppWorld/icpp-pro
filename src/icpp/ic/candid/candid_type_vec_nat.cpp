// The class for the Candid Type: vec

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeVecNat::CandidTypeVecNat() : CandidTypeVecBase() {}

CandidTypeVecNat::CandidTypeVecNat(std::vector<__uint128_t> *p_v)
    : CandidTypeVecBase() {}

CandidTypeVecNat::CandidTypeVecNat(const std::vector<__uint128_t> v)
    : CandidTypeVecBase() {}

CandidTypeVecNat::~CandidTypeVecNat() {}

void CandidTypeVecNat::set_content_type() {}

void CandidTypeVecNat::encode_M() {}

bool CandidTypeVecNat::decode_M(VecBytes B, __uint128_t &offset,
                                std::string &parse_error) {
  return false;
}