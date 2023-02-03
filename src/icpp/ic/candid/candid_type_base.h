// The base class for all Candid Types

#pragma once

#include <string>
#include <vector>

#include "candid.h"

#include "candid_opcode.h"
#include "vec_bytes.h"

class CandidTypeBase {
public:
  CandidTypeBase();
  ~CandidTypeBase();

  virtual void trap_if_wrong_type_on_wire(const std::string &type_on_wire);
  int get_datatype_opcode() { return m_datatype_opcode; }
  uint8_t get_datatype_hex() { return m_datatype_hex; }
  std::string get_datatype_textual() { return m_datatype_textual; };
  VecBytes get_T() { return m_T; }
  VecBytes get_I() { return m_I; }
  VecBytes get_M() { return m_M; }

  uint32_t idl_hash(const std::string &s);

  // Provide a less than operator to enable sorting
  friend bool operator<(CandidTypeBase const &lhs, CandidTypeBase const &rhs) {
    return lhs.m_datatype_opcode < rhs.m_datatype_opcode;
  }

  // Virtual method to be implemented by the <comptype> CandidTypes
  // Non <comptype> should not call this method.
  virtual bool decode_T(const VecBytes B, __uint128_t &offset,
                        std::string &parse_error);

  // Virtual method to be implemented by all CandidTypes
  virtual bool decode_M(VecBytes B, __uint128_t &offset,
                        std::string &parse_error,
                        CandidTypeBase *p_expected = nullptr);

protected:
  // The datatype
  int m_datatype_opcode{0};
  uint8_t m_datatype_hex{0x00};
  std::string m_datatype_textual{""};

  // The encoded byte vector for the Type Table
  VecBytes m_T;

  // The encoded byte vector for the Candid Type
  VecBytes m_I;

  // The encoded byte vector for the value
  VecBytes m_M;
};