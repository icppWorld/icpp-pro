// The base class for the Candid Type: vec

#include "candid.h"
#include "candid_opcode.h"
#include "pro.h"

#include <cassert>

#include "ic_api.h"

CandidTypeVecBase::CandidTypeVecBase() : CandidTypeBase() {
  Pro().exit_if_not_pro();
}

CandidTypeVecBase::~CandidTypeVecBase() {}

void CandidTypeVecBase::initialize() {}

void CandidTypeVecBase::set_datatype() {}

void CandidTypeVecBase::encode_T() {}

bool CandidTypeVecBase::decode_T(VecBytes B, __uint128_t &offset,
                                 std::string &parse_error) {
  return false;
}

void CandidTypeVecBase::encode_I() {}
