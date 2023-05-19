// Candid deserialization class
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results

#include "candid.h"
#include "candid_assert.h"
#include "candid_opcode.h"

#include "ic_api.h"

CandidDeserialize::CandidDeserialize() { deserialize(); }
CandidDeserialize::CandidDeserialize(const VecBytes &B,
                                     std::vector<CandidType> A) {
  m_A = A;
  m_B = B;
  m_hex_string = ""; // TOdo
  deserialize();
}
CandidDeserialize::CandidDeserialize(const std::string hex_string,
                                     std::vector<CandidType> A) {
  m_A = A;
  m_hex_string = hex_string;
  m_B.store_hex_string(hex_string);
  deserialize();
}
CandidDeserialize::~CandidDeserialize() {}

void CandidDeserialize::deserialize() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#deserialisation
  // Deserialisation at an expected type sequence (<t'>,*) proceeds by
  //
  // (1) checking for the magic number DIDL
  // (2) using the inverse of the T function to decode the type definitions (<t>,*)
  // (3) check that (<t>,*) <: (<t'>,*), else fail
  // (4) using the inverse of the M function, indexed by (<t>,*), to decode the values (<v>,*)
  // (5) use the coercion function C[(<t>,*) <: (<t'>,*)]((<v>,*)) to understand the decoded values at the expected type.
  //

  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results
  // B(kv* : <datatype>*) =
  // i8('D') i8('I') i8('D') i8('L')      magic number
  // T*(<comptype>*)                      type definition table
  // I*(<datatype>*)                      types of the argument list
  // M(kv* : <datatype>*)                 values of argument list

  __uint128_t B_offset = 0;

  // -------------------------------------------------------------------------------------
  // (1) checking for the magic number DIDL
  m_B.trap_if_vec_does_not_start_with_DIDL();
  B_offset = 4;

  // -------------------------------------------------------------------------------------
  // (2) using the inverse of the T function to decode the type definitions (<t>,*)
  //
  // T*(<comptype>*)                      type definition table
  // Write the unique TypeTables the comptypes:
  // (-) Constructed Types (opt, vec, record, variant)
  // (-) Reference Types (func, service)

  // Parse the number of unique type tables
  __uint128_t num_typetables;

  {
    __uint128_t B_offset_start = B_offset;
    std::string parse_error;
    __uint128_t numbytes;
    if (m_B.parse_uleb128(B_offset, num_typetables, numbytes, parse_error)) {
      std::string to_be_parsed =
          "Number of unique type tables, T*(<comptype>*)";
      trap_with_parse_error(B_offset_start, B_offset, to_be_parsed,
                            parse_error);
    }
  }

  // Parse the unique type tables
  for (size_t i = 0; i < num_typetables; ++i) {
    CandidTypeTable type_table = CandidTypeTable(m_B, B_offset);
    m_typetables.push_back(type_table);
  }

  // -------------------------------------------------------------------------------------
  // I*(<datatype>*)                      types of the argument list
  //                                      - Opcode for <primtype>
  //                                      or
  //                                      - TypeTable index for <comptype>

  m_args_datatypes.clear();
  m_args_datatypes_offset_start.clear();
  m_args_datatypes_offset_end.clear();

  // Parse the number of arguments in the byte stream
  __uint128_t num_args;

  {
    __uint128_t B_offset_start = B_offset;
    std::string parse_error;
    __uint128_t numbytes;
    if (m_B.parse_uleb128(B_offset, num_args, numbytes, parse_error)) {
      std::string to_be_parsed = "Number of arguments, I*(<datatype>*)";
      trap_with_parse_error(B_offset_start, B_offset, to_be_parsed,
                            parse_error);
    }
  }

  if (num_args != m_A.size()) {
    std::string msg;
    msg.append("ERROR: wrong number of arguments on wire.\n");
    msg.append("       Expected number of arguments:" +
               std::to_string(m_A.size()) + "\n");
    msg.append("       Number of arguments on wire :" +
               IC_API::to_string_128(num_args));
    IC_API::trap(msg);
  }

  for (size_t i = 0; i < num_args; ++i) {
    __uint128_t B_offset_start = B_offset;
    std::string parse_error;
    __uint128_t numbytes;
    __int128_t datatype;
    if (m_B.parse_sleb128(B_offset, datatype, numbytes, parse_error)) {
      std::string to_be_parsed = "datatype, I*(<datatype>*)";
      trap_with_parse_error(B_offset_start, B_offset, to_be_parsed,
                            parse_error);
    }
    m_args_datatypes.push_back(int(datatype));
    m_args_datatypes_offset_start.push_back(B_offset_start);
    m_args_datatypes_offset_end.push_back(B_offset);
  }

  // -------------------------------------------------------------------------------------
  // (3) check that (<t>,*) <: (<t'>,*), else fail
  //
  //  (<t>,*)  = found type definitions
  //  (<t'>,*) = expected type definitions, provided via m_pA
  //  <:       = the found type definitions (<t>,*) are structural subtypes of the expected type sequence (<t'>,*)
  //
  //             https://github.com/dfinity/candid/blob/master/spec/Candid.md#upgrading-and-subtyping
  //
  std::vector<int> v_opcode_found;
  for (size_t i = 0; i < num_args; ++i) {
    __uint128_t B_offset_start = B_offset;
    std::string parse_error;
    __uint128_t numbytes;
    if (m_args_datatypes[i] >= 0) {
      // <comptype>

      // the datatype is an index into the typetable of the byte stream
      size_t type_table_index = m_args_datatypes[i];
      CandidTypeTable type_table = m_typetables[type_table_index];

      // Verify that the type_table found on the wire is the same as the expected type_table
      int opcode_found = type_table.get_opcode();
      v_opcode_found.push_back(opcode_found);

      int opcode_expected =
          std::visit([](auto &&c) { return c.get_datatype_opcode(); }, m_A[i]);

      if (opcode_found != opcode_expected) {
        if (opcode_expected < 0) {
          std::string msg;
          msg.append(
              "ERROR: expecting an Opcode, but a type table reference was found on wire instead.\n");
          msg.append("       Argument index    : " + std::to_string(i) + "\n");
          msg.append("       Bytes offset start: " +
                     IC_API::to_string_128(m_args_datatypes_offset_start[i]) +
                     "\n");
          msg.append("       Bytes offset end  : " +
                     IC_API::to_string_128(m_args_datatypes_offset_end[i]) +
                     "\n");
          msg.append(
              "       Expecting opcode  :" + std::to_string(opcode_expected) +
              " (" + CandidOpcode().name_from_opcode(opcode_expected) + ")" +
              "\n");
          msg.append("       Found type table    :" +
                     std::to_string(opcode_found));
          IC_API::trap(msg);
        } else {
          std::string msg;
          msg.append("ERROR: wrong type table reference found on wire.\n");
          msg.append("       Argument index: " + std::to_string(i) + "\n");
          msg.append("       Bytes offset start: " +
                     IC_API::to_string_128(m_args_datatypes_offset_start[i]) +
                     "\n");
          msg.append("       Bytes offset end  : " +
                     IC_API::to_string_128(m_args_datatypes_offset_end[i]) +
                     "\n");
          msg.append("       Expecting type table:" +
                     std::to_string(opcode_expected) + "\n");
          msg.append("       Found type table    :" +
                     std::to_string(opcode_found));
          IC_API::trap(msg);
        }
      }
      if (opcode_found == CandidOpcode().Record) {
        CandidTypeRecord *p_wire =
            get_if<CandidTypeRecord>(type_table.get_candidType_ptr());
        CandidTypeRecord *p_expected = get_if<CandidTypeRecord>(&m_A[i]);

        // Trap if type table does not match the wire
        p_expected->check_type_table(p_wire);
      } else if (opcode_found == CandidOpcode().Variant) {
        CandidTypeVariant *p_wire =
            get_if<CandidTypeVariant>(type_table.get_candidType_ptr());
        CandidTypeVariant *p_expected = get_if<CandidTypeVariant>(&m_A[i]);
        p_expected->check_type_table(p_wire);
      } else if (opcode_found == CandidOpcode().Vec ||
                 opcode_found == CandidOpcode().Opt) {

        int content_opcode_found =
            std::visit([](auto &&c) { return c.get_content_type_opcode(); },
                       *type_table.get_candidType_ptr());
        int content_opcode_expected = std::visit(
            [](auto &&c) { return c.get_content_type_opcode(); }, m_A[i]);

        if (content_opcode_found != content_opcode_expected) {
          std::string msg;
          if (opcode_found == CandidOpcode().Vec) {
            msg.append(
                "ERROR: Vector with wrong content opcode found on wire.\n");
          } else if (opcode_found == CandidOpcode().Opt) {
            msg.append("ERROR: Opt with wrong content opcode found on wire.\n");
          } else {
            msg.append(
                "ERROR: const of unknown type with wrong content opcode found on wire.\n");
          }

          msg.append("       Argument index: " + std::to_string(i) + "\n");
          msg.append("       Bytes offset start: " +
                     IC_API::to_string_128(m_args_datatypes_offset_start[i]) +
                     "\n");
          msg.append("       Bytes offset end  : " +
                     IC_API::to_string_128(m_args_datatypes_offset_end[i]) +
                     "\n");
          msg.append("       Expecting content opcode:" +
                     std::to_string(content_opcode_expected) + " (" +
                     CandidOpcode().name_from_opcode(content_opcode_expected) +
                     ")" + "\n");
          msg.append("       Found content opcode    :" +
                     std::to_string(content_opcode_found) + " (" +
                     CandidOpcode().name_from_opcode(content_opcode_found) +
                     ")" + "\n");
          IC_API::trap(msg);
        }
      } else {
        IC_API::trap(
            "ERROR: Deserialization not yet implemented for this constype");
      }

    } else {
      // <primptype>

      // There is no type-table. The datatype is an Opcode
      int opcode_found = m_args_datatypes[i];
      v_opcode_found.push_back(opcode_found);

      int opcode_expected =
          std::visit([](auto &&c) { return c.get_datatype_opcode(); }, m_A[i]);

      if (opcode_found != opcode_expected) {
        if (opcode_found == CandidOpcode().Null &&
            opcode_expected == CandidOpcode().Opt) {
          // This is ok. A null is passed for an Opt
        } else {
          // TODO:  IMPLEMENT CHECK ON COVARIANCE/CONTRAVARIANCE
          std::string msg;
          msg.append("ERROR: wrong opcode found on wire.\n");
          msg.append("       Argument index: " + std::to_string(i) + "\n");
          msg.append("       Bytes offset start: " +
                     IC_API::to_string_128(m_args_datatypes_offset_start[i]) +
                     "\n");
          msg.append("       Bytes offset end  : " +
                     IC_API::to_string_128(m_args_datatypes_offset_end[i]) +
                     "\n");
          msg.append(
              "       Expecting opcode:" + std::to_string(opcode_expected) +
              " (" + CandidOpcode().name_from_opcode(opcode_expected) + ")" +
              "\n");
          msg.append("       Found opcode    :" + std::to_string(opcode_found) +
                     " (" + CandidOpcode().name_from_opcode(opcode_found) +
                     ")");
          IC_API::trap(msg);
        }
      }
    }
  }
  // (4) using the inverse of the M function, indexed by (<t>,*), to decode the values (<v>,*)
  //
  // (5) use the coercion function C[(<t>,*) <: (<t'>,*)]((<v>,*)) to understand the decoded values at the expected type.
  // M(kv* : <datatype>*)                 values of argument list

  for (size_t i = 0; i < num_args; ++i) {
    if (v_opcode_found[i] == CandidOpcode().Null) {
      // There is no value to decode
      continue;
    }
    __uint128_t B_offset_start = B_offset;
    std::string parse_error = "";
    __uint128_t numbytes;

    if (std::visit(
            [&](auto &&c) { return c.decode_M(m_B, B_offset, parse_error); },
            m_A[i])) {
      std::string to_be_parsed =
          "Values (decoding M) for argument at index " + std::to_string(i);
      CandidDeserialize::trap_with_parse_error(B_offset_start, B_offset,
                                               to_be_parsed, parse_error);
    }
  }

  // Append R
  // Was never implemented int Candid, although it is still in the spec
  // https://www.joachim-breitner.de/blog/786-A_Candid_explainer__Quirks
}

// Assert candid VecBytes against a string in "hex" format (didc encode)
int CandidDeserialize::assert_candid(const std::string &candid_expected,
                                     const bool &assert_value) {
  return CandidAssert::assert_candid(m_B, candid_expected, assert_value);
}

// Trap with parse error message
void CandidDeserialize::trap_with_parse_error(const __uint128_t &B_offset_start,
                                              const __uint128_t &B_offset,
                                              const std::string &to_be_parsed,
                                              const std::string &parse_error) {
  std::string msg;
  msg.append("ERROR: decoding of Candid byte stream failed.\n");
  msg.append("       trying to extract: " + to_be_parsed + "\n");
  msg.append("       parsing error:" + parse_error + "\n");
  msg.append("       byte offset:" + IC_API::to_string_128(B_offset_start) +
             "\n");
  IC_API::trap(msg);
}