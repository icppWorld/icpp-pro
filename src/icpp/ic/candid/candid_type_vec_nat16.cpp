// The class for the Candid Type: vec

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeVecNat16::CandidTypeVecNat16() : CandidTypeVecBase() {}

CandidTypeVecNat16::CandidTypeVecNat16(std::vector<uint16_t> *p_v)
    : CandidTypeVecBase() {}

CandidTypeVecNat16::CandidTypeVecNat16(const std::vector<uint16_t> v)
    : CandidTypeVecBase() {}

CandidTypeVecNat16::~CandidTypeVecNat16() {}

void CandidTypeVecNat16::set_content_type() {}

void CandidTypeVecNat16::encode_M() {}

bool CandidTypeVecNat16::decode_M(VecBytes B, __uint128_t &offset,
                                  std::string &parse_error) {
  return false;
}