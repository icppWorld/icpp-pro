// The class for the Candid Type: opt

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeOptNat64::CandidTypeOptNat64() : CandidTypeOptBase() {}

CandidTypeOptNat64::CandidTypeOptNat64(std::optional<uint64_t> *p_v)
    : CandidTypeOptBase() {}

CandidTypeOptNat64::CandidTypeOptNat64(const std::optional<uint64_t> v)
    : CandidTypeOptBase() {}

CandidTypeOptNat64::~CandidTypeOptNat64() {}

void CandidTypeOptNat64::set_content_type() {}

void CandidTypeOptNat64::encode_M() {}

bool CandidTypeOptNat64::decode_M(VecBytes B, __uint128_t &offset,
                                  std::string &parse_error) {
  return false;
}