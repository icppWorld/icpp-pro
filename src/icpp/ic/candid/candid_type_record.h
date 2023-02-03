// The class for the Primitive Candid Type: text

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeRecord : public CandidTypeBase {
public:
  // Constructors
  CandidTypeRecord();

  // Destructor
  ~CandidTypeRecord();

  void append(uint32_t field_id, CandidType field);
  void append(std::string field_name, CandidType field);
  void append(CandidType field);

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);
  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error,
                CandidTypeRecord *p_expected);

protected:
  void _append(uint32_t field_id, std::string field_name, CandidType field);

  void set_datatype();
  void encode_T();
  void encode_I();
  void encode_M();

  std::vector<uint32_t> m_field_ids; // id | hash
  std::vector<std::string> m_field_names;
  std::vector<int> m_field_datatypes;
  std::vector<CandidType> m_fields;
};