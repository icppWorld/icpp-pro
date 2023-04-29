// The class for the Candid Type: vec

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeVecNat64::CandidTypeVecNat64() : CandidTypeVecBase() {}

CandidTypeVecNat64::CandidTypeVecNat64(std::vector<uint64_t> *p_v)
    : CandidTypeVecBase() {}

CandidTypeVecNat64::CandidTypeVecNat64(const std::vector<uint64_t> v)
    : CandidTypeVecBase() {}

CandidTypeVecNat64::~CandidTypeVecNat64() {}

void CandidTypeVecNat64::set_content_type() {}

void CandidTypeVecNat64::encode_M() {}

bool CandidTypeVecNat64::decode_M(VecBytes B, __uint128_t &offset,
                                  std::string &parse_error) {
  return false;
}