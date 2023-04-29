// The class for the Candid Type: vec

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeVecNat32::CandidTypeVecNat32() : CandidTypeVecBase() {}

CandidTypeVecNat32::CandidTypeVecNat32(std::vector<uint32_t> *p_v)
    : CandidTypeVecBase() {}

CandidTypeVecNat32::CandidTypeVecNat32(const std::vector<uint32_t> v)
    : CandidTypeVecBase() {}

CandidTypeVecNat32::~CandidTypeVecNat32() {}

void CandidTypeVecNat32::set_content_type() {}

void CandidTypeVecNat32::encode_M() {}

bool CandidTypeVecNat32::decode_M(VecBytes B, __uint128_t &offset,
                                  std::string &parse_error) {
  return false;
}