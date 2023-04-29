// The class for the Candid Type: opt

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"

CandidTypeOptFloat64::CandidTypeOptFloat64() : CandidTypeOptBase() {
  std::optional<double> v;
}

CandidTypeOptFloat64::CandidTypeOptFloat64(std::optional<double> *p_v)
    : CandidTypeOptBase() {}

CandidTypeOptFloat64::CandidTypeOptFloat64(const std::optional<double> v)
    : CandidTypeOptBase() {}

CandidTypeOptFloat64::~CandidTypeOptFloat64() {}

void CandidTypeOptFloat64::set_content_type() {}

void CandidTypeOptFloat64::encode_M() {}

bool CandidTypeOptFloat64::decode_M(VecBytes B, __uint128_t &offset,
                                    std::string &parse_error) {
  return false;
}