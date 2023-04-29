// The class for the Candid Type: opt

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeOptNat::CandidTypeOptNat() : CandidTypeOptBase() {}

CandidTypeOptNat::CandidTypeOptNat(std::optional<__uint128_t> *p_v)
    : CandidTypeOptBase() {}

CandidTypeOptNat::CandidTypeOptNat(const std::optional<__uint128_t> v)
    : CandidTypeOptBase() {}

CandidTypeOptNat::~CandidTypeOptNat() {}

void CandidTypeOptNat::set_content_type() {}

void CandidTypeOptNat::encode_M() {}

bool CandidTypeOptNat::decode_M(VecBytes B, __uint128_t &offset,
                                std::string &parse_error) {
  return false;
}