// The base class for all Candid Types

#include "candid.h"

#include <charconv>
#include <cmath>
#include <system_error>

#include "ic_api.h"

CandidTypeBase::CandidTypeBase() {}

CandidTypeBase::~CandidTypeBase() {}

void CandidTypeBase::trap_if_wrong_type_on_wire(
    const std::string &type_on_wire) {
  if (type_on_wire != m_datatype_textual) {
    std::string msg;
    msg.append("ERROR: wrong Candid type on wire.\n");
    msg.append("       Candid type on wire  = " + type_on_wire + "\n");
    msg.append("       Expected Candid type = " + m_datatype_textual);
    IC_API::trap(msg);
  };
}

// Virtual method to be implemented by the <comptype> CandidTypes to support deserialization
// Non <comptype> should not call this method.
bool CandidTypeBase::decode_T(const VecBytes B, __uint128_t &offset,
                              std::string &parse_error) {
  IC_API::trap("ERROR: decode_T not implemented...");
  return true;
}

// Virtual method to be implemented by all CandidTypes to support deserialization
bool CandidTypeBase::decode_M(const VecBytes B, __uint128_t &offset,
                              std::string &parse_error,
                              CandidTypeBase *p_expected) {
  IC_API::trap("ERROR: decode_M not implemented...");
  return true;
}

// https://github.com/dfinity/candid/blob/master/spec/Candid.md#records
uint32_t CandidTypeBase::idl_hash(const std::string &s) {
  uint32_t hash{};

  // hash(id) = ( Sum_(i=0..k) utf8(id)[i] * 223^(k-i) ) mod 2^32 where k = |utf8(id)|-1
  hash = 0;
  uint64_t p = 4294967296; // std::pow(2,32)
  for (int c : s) {
    hash = (hash * 223 + c) % p;
  }

  return hash;
}
