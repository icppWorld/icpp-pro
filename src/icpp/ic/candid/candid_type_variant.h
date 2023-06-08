// The class for the Candid Type: variant

#pragma once

#include <cstring>

#include "candid.h"

class CandidTypeVariant : public CandidTypeBase {
public:
  // Constructors
  CandidTypeVariant();
  CandidTypeVariant(std::string *p_label);
  CandidTypeVariant(const std::string label);
  CandidTypeVariant(const std::string label, const CandidType field);

  // Destructor
  ~CandidTypeVariant();

  void append(uint32_t field_id, CandidType field);
  void append(std::string field_name, CandidType field);
  void append(CandidType field);

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);
  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);

  void check_type_table(const CandidTypeVariant *p_from_wire);

  std::string get_label() { return m_label; }

protected:
  void set_p_label(std::string *p_label) { m_p_label = p_label; };
  void _append(uint32_t field_id, std::string field_name, CandidType field);

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
  void encode_M();

  std::vector<uint32_t> m_field_ids; // id | hash
  std::vector<std::string> m_field_names;
  std::vector<int> m_field_datatypes;
  std::vector<CandidType> m_fields;

  // Label & id (hash) of the field that contains the Variant's data
  std::string m_label{""};
  __uint128_t m_variant_index{0};
  bool m_label_value_set{false};

  // Pointer to the label passed in by caller during deserialization
  std::string *m_p_label{nullptr};

  // To help with decoding checks
  std::vector<uint32_t> m_field_ids_wire; // id | hash
  std::vector<int> m_field_datatypes_wire;
  __uint128_t m_variant_index_wire{0};
};
