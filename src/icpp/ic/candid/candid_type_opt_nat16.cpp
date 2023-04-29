// The class for the Candid Type: opt

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeOptNat16::CandidTypeOptNat16() : CandidTypeOptBase() {}

CandidTypeOptNat16::CandidTypeOptNat16(std::optional<uint16_t> *p_v)
    : CandidTypeOptBase() {}

CandidTypeOptNat16::CandidTypeOptNat16(const std::optional<uint16_t> v)
    : CandidTypeOptBase() {}

CandidTypeOptNat16::~CandidTypeOptNat16() {}

void CandidTypeOptNat16::set_content_type() {}

void CandidTypeOptNat16::encode_M() {}

bool CandidTypeOptNat16::decode_M(VecBytes B, __uint128_t &offset,
                                  std::string &parse_error) {
  return false;
}