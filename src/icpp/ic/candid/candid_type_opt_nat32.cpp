// The class for the Candid Type: opt

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeOptNat32::CandidTypeOptNat32() : CandidTypeOptBase() {}

CandidTypeOptNat32::CandidTypeOptNat32(std::optional<uint32_t> *p_v)
    : CandidTypeOptBase() {}

CandidTypeOptNat32::CandidTypeOptNat32(const std::optional<uint32_t> v)
    : CandidTypeOptBase() {}

CandidTypeOptNat32::~CandidTypeOptNat32() {}

void CandidTypeOptNat32::set_content_type() {}

void CandidTypeOptNat32::encode_M() {}

bool CandidTypeOptNat32::decode_M(VecBytes B, __uint128_t &offset,
                                  std::string &parse_error) {
  return false;
}