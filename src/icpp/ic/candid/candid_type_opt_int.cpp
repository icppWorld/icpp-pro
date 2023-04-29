// The class for the Candid Type: opt

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeOptInt::CandidTypeOptInt() : CandidTypeOptBase() {}

CandidTypeOptInt::CandidTypeOptInt(std::optional<__int128_t> *p_v)
    : CandidTypeOptBase() {}

CandidTypeOptInt::CandidTypeOptInt(const std::optional<__int128_t> v)
    : CandidTypeOptBase() {}

CandidTypeOptInt::~CandidTypeOptInt() {}

void CandidTypeOptInt::set_content_type() {}

void CandidTypeOptInt::encode_M() {}

bool CandidTypeOptInt::decode_M(VecBytes B, __uint128_t &offset,
                                std::string &parse_error) {
  return false;
}