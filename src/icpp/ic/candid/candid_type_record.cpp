// The class for the Candid Type: Record

#include "candid.h"
#include "candid_opcode.h"
#include "pro.h"

#include "ic_api.h"

#include <algorithm>
#include <cassert>
#include <utility>

CandidTypeRecord::CandidTypeRecord() : CandidTypeBase() {
  Pro().exit_if_not_pro();
}

CandidTypeRecord::~CandidTypeRecord() {}

void CandidTypeRecord::set_datatype() {}

void CandidTypeRecord::append(uint32_t field_id, CandidType field) {}

void CandidTypeRecord::append(std::string field_name, CandidType field) {}

void CandidTypeRecord::append(CandidType field) {}

void CandidTypeRecord::_append(uint32_t field_id, std::string field_name,
                               CandidType field) {}

void CandidTypeRecord::encode_T() {}

bool CandidTypeRecord::decode_T(VecBytes B, __uint128_t &offset,
                                std::string &parse_error) {
  return false;
}

void CandidTypeRecord::encode_I() {}

void CandidTypeRecord::encode_M() {}

bool CandidTypeRecord::decode_M(VecBytes B, __uint128_t &offset,
                                std::string &parse_error) {
  return false;
}

void CandidTypeRecord::check_type_table(const CandidTypeRecord *p_from_wire) {}
