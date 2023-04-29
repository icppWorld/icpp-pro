// The base class for the Candid Type: opt

#include "candid.h"
#include "candid_opcode.h"

#include <cassert>

#include "ic_api.h"
#include "pro.h"

CandidTypeOptBase::CandidTypeOptBase() : CandidTypeBase() {
  Pro().exit_if_not_pro();
}

CandidTypeOptBase::~CandidTypeOptBase() {}

void CandidTypeOptBase::initialize() {}

void CandidTypeOptBase::set_datatype() {}

void CandidTypeOptBase::encode_T() {}

bool CandidTypeOptBase::decode_T(VecBytes B, __uint128_t &offset,
                                 std::string &parse_error) {
  return false;
}

void CandidTypeOptBase::encode_I() {}
