// The class for the Candid Type: opt

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeOptInt64::CandidTypeOptInt64() : CandidTypeOptBase() {}

CandidTypeOptInt64::CandidTypeOptInt64(std::optional<int64_t> *p_v)
    : CandidTypeOptBase() {}

CandidTypeOptInt64::CandidTypeOptInt64(const std::optional<int64_t> v)
    : CandidTypeOptBase() {}

CandidTypeOptInt64::~CandidTypeOptInt64() {}

void CandidTypeOptInt64::set_content_type() {}

void CandidTypeOptInt64::encode_M() {}

bool CandidTypeOptInt64::decode_M(VecBytes B, __uint128_t &offset,
                                  std::string &parse_error) {
  return false;
}