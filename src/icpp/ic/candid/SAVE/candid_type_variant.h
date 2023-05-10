// The class for the Candid Type: variant

#pragma once

#include <algorithm>
#include <cassert>
#include <cstring>
#include <utility>
#include <variant>

#include "candid.h"
#include "candid_opcode.h"
#include "pro.h"

#include "ic_api.h"

/////////////////
// Declaration //
/////////////////

template <typename... V> class CandidTypeVariant : public CandidTypeBase {
public:
  // Constructors
  CandidTypeVariant();
  CandidTypeVariant(std::variant<V...> *v);
  CandidTypeVariant(const std::variant<V...> v);

  // Destructor
  ~CandidTypeVariant();

  // bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);
  // bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);

  // void check_type_table(const CandidTypeVariant *p_from_wire);

protected:
  void set_pv(std::variant<V...> *v) { m_pv = v; }
  void set_v(const std::variant<V...> &v) { m_v = v; }
  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
  void encode_M();

  // TODO: REMOVE THESE RECORD FIELDS...
  // void _append(uint32_t field_id, std::string field_name, CandidType field);
  // std::vector<uint32_t> m_field_ids; // id | hash
  // std::vector<std::string> m_field_names;
  // std::vector<int> m_field_datatypes;
  // std::vector<CandidType> m_fields;
  // // To help with decoding checks
  // std::vector<int> m_field_datatypes_wire;

  std::variant<V...> m_v;
  std::variant<V...> *m_pv{nullptr};
};

////////////////////
// Implementation //
////////////////////

template <typename... V>
CandidTypeVariant<V>::CandidTypeVariant() : CandidTypeBase() {
  Pro().exit_if_not_pro();
  std::variant<V...> v;
  set_v(v);
  initialize();
}

// This constructor allows for setting the value during Deserialization
template <typename... V>
CandidTypeVariant<V>::CandidTypeVariant(std::variant<V...> *p_v)
    : CandidTypeBase() {
  Pro().exit_if_not_pro();
  set_pv(p_v);

  const std::variant<V...> v = const_cast<std::variant<V...> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
template <typename... V>
CandidTypeVariant<V>::CandidTypeVariant(const std::variant<V...> v)
    : CandidTypeBase() {
  Pro().exit_if_not_pro();
  set_v(v);
  initialize();
}

template <typename... V> CandidTypeVariant<V>::~CandidTypeVariant() {}

// Initialize things
template <typename... V>
void CandidTypeVariant<V>::initialize(const std::variant<V...> &v) {
  set_datatype();
  encode_T();
  encode_I();
  encode_M();
}

template <typename... V> void CandidTypeVariant<V>::set_datatype() {
  m_datatype_opcode = CandidOpcode().Variant;
  m_datatype_hex = OpcodeHex().Variant;
  m_datatype_textual = OpcodeTextual().Variant;
}

// // Tuple notation with uint32_t field_id
// template <typename... V>
// void CandidTypeVariant<V>::append(uint32_t field_id, CandidType field) {
//   std::string s = "";
//   _append(field_id, s, field);
// }

// // Record notation with string name -> hash name into uint32_t field_id
// template <typename... V>
// void CandidTypeVariant<V>::append(std::string field_name, CandidType field) {
//   uint32_t field_id = idl_hash(field_name);
//   _append(field_id, field_name, field);
// }

// // Tuple notation without field_id -> generate sequential uint32_t field_id
// template <typename... V> void CandidTypeVariant<V>::append(CandidType field) {
//   if (m_fields.size() == 0) {
//     uint32_t field_id = 0;
//     append(field_id, field);
//   }

//   if (std::in_range<std::uint32_t>(m_field_ids.back() + 1)) {
//     uint32_t field_id = m_field_ids.back() + 1;
//     append(field_id, field);
//   } else {
//     std::string msg;
//     msg.append("ERROR: field_id outside range of uint32_t\n");
//     msg.append("       id of previous field in record: " +
//                std::to_string(m_field_ids.back()) + "\n");
//     msg.append(
//         "       because no field_id was specified for current field, we want to increase it by 1");
//     IC_API::trap(msg);
//   }
// }

// template <typename... V>
// void CandidTypeVariant<V>::_append(uint32_t field_id, std::string field_name,
//                                    CandidType field) {
//   // Check if field with identical hash already exists
//   auto iter = std::find(begin(m_field_ids), end(m_field_ids), field_id);
//   if (iter != end(m_field_ids)) {
//     auto i = std::distance(begin(m_field_ids), iter);
//     std::string msg;
//     msg.append("ERROR: record already has a field with the same field_id.\n");
//     msg.append("       field id (hash): " + std::to_string(field_id) + "\n");
//     msg.append("       field name 1   : " + m_field_names[i] + "\n");
//     msg.append("       field name 2   : " + field_name + "\n");
//     IC_API::trap(msg);
//   }

//   // Add the field
//   m_field_ids.push_back(field_id);
//   m_field_names.push_back(field_name);
//   int datatype =
//       std::visit([](auto &&c) { return c.get_datatype_opcode(); }, field);
//   m_field_datatypes.push_back(datatype);
//   m_fields.push_back(field);

//   // Sort by field_id (hash)
//   for (std::size_t i = 0; i < m_field_ids.size(); ++i) {
//     for (std::size_t j = i + 1; j < m_field_ids.size(); ++j) {
//       if (m_field_ids[i] > m_field_ids[j]) {
//         auto temp_field_id = std::move(m_field_ids[i]);
//         m_field_ids[i] = std::move(m_field_ids[j]);
//         m_field_ids[j] = std::move(temp_field_id);

//         auto temp_field_name = std::move(m_field_names[i]);
//         m_field_names[i] = std::move(m_field_names[j]);
//         m_field_names[j] = std::move(temp_field_name);

//         auto temp_field = std::move(m_fields[i]);
//         m_fields[i] = std::move(m_fields[j]);
//         m_fields[j] = std::move(temp_field);

//         auto temp_field_datatype = std::move(m_field_datatypes[i]);
//         m_field_datatypes[i] = std::move(m_field_datatypes[j]);
//         m_field_datatypes[j] = std::move(temp_field_datatype);
//       }
//     }
//   }

//   encode_T();
//   encode_M();
// }

// Build the type table encoding
template <typename... V> void CandidTypeVariant<V>::encode_T() {
  IC_API::trap("TODO: encode_T");
  // m_T.clear();

  // m_T.append_byte((std::byte)m_datatype_hex);
  // m_T.append_uleb128(__uint128_t(m_fields.size()));
  // for (size_t i = 0; i < m_fields.size(); ++i) {
  //   // id or hash of the record field, append without packing into a fixed width
  //   m_T.append_uleb128(__uint128_t(m_field_ids[i]));

  //   // data type of the record field
  //   VecBytes I = std::visit([](auto &&c) { return c.get_I(); }, m_fields[i]);
  //   for (std::byte b : I.vec()) {
  //     m_T.append_byte(b);
  //   }
  // }
}

// Decode the type table, starting at & updating offset
template <typename... V>
bool CandidTypeVariant<V>::decode_T(VecBytes B, __uint128_t &offset,
                                    std::string &parse_error) {
  IC_API::trap("TODO: decode_T");
  // __uint128_t len = B.size() - offset;

  // m_field_ids.clear();
  // m_field_names.clear();
  // m_fields.clear();
  // m_field_datatypes.clear();

  // __uint128_t num_fields;
  // __uint128_t numbytes;
  // if (B.parse_uleb128(offset, num_fields, numbytes, parse_error)) {
  //   return true;
  // }

  // for (size_t i = 0; i < num_fields; ++i) {
  //   // id or hash of the record field
  //   __uint128_t field_id;
  //   __uint128_t numbytes;
  //   if (B.parse_uleb128(offset, field_id, numbytes, parse_error)) {
  //     return true;
  //   }
  //   m_field_ids.push_back(field_id);
  //   m_field_names.push_back("");

  //   // Get the datatype of the type table
  //   __uint128_t offset_start = offset;
  //   std::string parse_error;
  //   __int128_t datatype;
  //   numbytes = 0;
  //   if (B.parse_sleb128(offset, datatype, numbytes, parse_error)) {
  //     std::string to_be_parsed = "Type table: datatype";
  //     CandidDeserialize::trap_with_parse_error(offset_start, offset,
  //                                              to_be_parsed, parse_error);
  //   }
  //   m_field_datatypes.push_back(int(datatype));

  //   // Create a CandidType instance for the field
  //   CandidType c;
  //   if (int(datatype) == CandidOpcode().Vec) {
  //     // for a Vec, the typetable is simple the datatype of it's content
  //     offset_start = offset;
  //     parse_error = "";
  //     __int128_t content_opcode;
  //     if (B.parse_sleb128(offset, content_opcode, numbytes, parse_error)) {
  //       std::string to_be_parsed =
  //           "Type table: a record field of type Vec -> the Vec's content type";
  //       CandidDeserialize::trap_with_parse_error(offset_start, offset,
  //                                                to_be_parsed, parse_error);
  //     }
  //     CandidOpcode().candid_type_vec_from_opcode(c, content_opcode);
  //   } else if (int(datatype) == CandidOpcode().Opt) {
  //     // for an Opt, the typetable is simple the datatype of it's content
  //     offset_start = offset;
  //     parse_error = "";
  //     __int128_t content_opcode;
  //     if (B.parse_sleb128(offset, content_opcode, numbytes, parse_error)) {
  //       std::string to_be_parsed =
  //           "Type table: a record field of type Opt -> the Opt's content type";
  //       CandidDeserialize::trap_with_parse_error(offset_start, offset,
  //                                                to_be_parsed, parse_error);
  //     }
  //     CandidOpcode().candid_type_opt_from_opcode(c, content_opcode);
  //   } else {
  //     // Decode type table using the CandidType variant's decode_T method.
  //     CandidOpcode().candid_type_from_opcode(c, datatype);
  //     parse_error = "";
  //     if (std::visit(
  //             [&](auto &&c) { return c.decode_T(B, offset, parse_error); },
  //             c)) {
  //       std::string to_be_parsed = "Type table: record field";
  //       CandidDeserialize::trap_with_parse_error(offset_start, offset,
  //                                                to_be_parsed, parse_error);
  //     }
  //   }
  //   // Store the CandidType in m_fields vector
  //   m_fields.push_back(c);
  // }
  return false;
}

// For variants, we set the Opcode, but note that it is not used during serialization.
// At serialization time, we use the index in the overall type table.
//
// Encode the datatype
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// For <constype>: the negative Opcode
template <typename... V> void CandidTypeVariant<V>::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}

// Build the value encoding
template <typename... V> void CandidTypeVariant<V>::encode_M() {
  IC_API::trap("TODO: encode_M");
  // m_M.clear();

  // for (CandidType field : m_fields) {
  //   VecBytes M = std::visit([](auto &&c) { return c.get_M(); }, field);
  //   for (std::byte b : M.vec()) {
  //     m_M.append_byte(b);
  //   }
  // }
}

// Decode the values, starting at & updating offset
template <typename... V>
bool CandidTypeVariant<V>::decode_M(VecBytes B, __uint128_t &offset,
                                    std::string &parse_error) {
  IC_API::trap("TODO: decode_M");
  // __uint128_t len = B.size() - offset;

  // for (size_t i = 0; i < m_fields.size(); ++i) {
  //   if (m_field_datatypes_wire[i] == CandidOpcode().Null) {
  //     // There is no value to decode
  //     continue;
  //   }
  //   int datatype = m_field_datatypes[i];
  //   if (CandidOpcode().is_primtype(datatype)) {
  //     parse_error = "";
  //     __uint128_t offset_start = offset;
  //     if (std::visit(
  //             [&](auto &&c) { return c.decode_M(B, offset, parse_error); },
  //             m_fields[i])) {
  //       std::string to_be_parsed = "Value for a Record field";
  //       CandidDeserialize::trap_with_parse_error(offset_start, offset,
  //                                                to_be_parsed, parse_error);
  //     }
  //   } else {
  //     IC_API::trap(
  //         "TODO: Implement decode for non primitive type as a record field, using recursion " +
  //         std::to_string(datatype) + std::string(__func__));
  //   }
  // }

  return false;
}

// Traps if the type table does not match the type table on the wire
template <typename... V>
void CandidTypeVariant<V>::check_type_table(
    const CandidTypeVariant *p_from_wire) {
  IC_API::trap("TODO: check_type_table");
  // m_field_datatypes_wire.clear();
  // for (size_t i = 0; i < m_fields.size(); ++i) {
  //   // id or hash of the record field
  //   uint32_t id = m_field_ids[i];
  //   uint32_t id_wire = p_from_wire->m_field_ids[i];
  //   if (id != id_wire) {
  //     std::string msg;
  //     msg.append("ERROR: the hashed id for the Record field at index " +
  //                std::to_string(i) + " is wrong on the wire.\n");
  //     msg.append("       expected value of the hashed id: " +
  //                VecBytes::my_uint128_to_string(__uint128_t(id)) + "\n");
  //     msg.append("       found on wire  : " +
  //                VecBytes::my_uint128_to_string(__uint128_t(id_wire)) + "\n");
  //     IC_API::trap(msg);
  //   }

  //   int datatype = m_field_datatypes[i];
  //   int datatype_wire = p_from_wire->m_field_datatypes[i];
  //   m_field_datatypes_wire.push_back(
  //       datatype_wire); // save it for use in decode_M

  //   if (datatype != datatype_wire) {
  //     if (datatype_wire == CandidOpcode().Null &&
  //         datatype == CandidOpcode().Opt) {
  //       // This is ok. A null is passed for an Opt
  //     } else {
  //       std::string msg;
  //       msg.append("ERROR: the datatype for the Record field at index " +
  //                  std::to_string(i) + " is wrong on the wire.\n");
  //       msg.append("       expected datatype: " + std::to_string(datatype) +
  //                  " (" + CandidOpcode().name_from_opcode(datatype) + ")" +
  //                  "\n");
  //       msg.append("       type on wire : " + std::to_string(datatype_wire) +
  //                  " (" + CandidOpcode().name_from_opcode(datatype_wire) + ")" +
  //                  "\n");
  //       IC_API::trap(msg);
  //     }
  //   }
  // }
}
