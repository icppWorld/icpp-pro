// Candid deserialization class
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results

#include "candid.h"
#include "candid_assert.h"

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
  //                                      - TypeTable index for <comptype>

  m_args_datatypes.clear();

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
  // (4) using the inverse of the M function, indexed by (<t>,*), to decode the values (<v>,*)
  //
  // (5) use the coercion function C[(<t>,*) <: (<t'>,*)]((<v>,*)) to understand the decoded values at the expected type.
  // M(kv* : <datatype>*)                 values of argument list

  for (size_t i = 0; i < num_args; ++i) {
    __uint128_t B_offset_start = B_offset;
    std::string parse_error;
    __uint128_t numbytes;
    if (m_args_datatypes[i] >= 0) {
      // <comptype>
      if (deserialize_values_comptype(B_offset, i, numbytes, parse_error)) {
        std::string to_be_parsed = "Values of argument " + std::to_string(i) +
                                   ", M(kv* : <datatype>*) )";
        trap_with_parse_error(B_offset_start, B_offset, to_be_parsed,
                              parse_error);
      }

    } else {
      // <primptype>
      if (deserialize_values_primtype(B_offset, i, numbytes, parse_error)) {
        std::string to_be_parsed = "Values of argument " + std::to_string(i) +
                                   ", M(kv* : <datatype>*) )";
        trap_with_parse_error(B_offset_start, B_offset, to_be_parsed,
                              parse_error);
      }
    }
  }

  // Append R
  // Was never implemented int Candid, although it is still in the spec
  // https://www.joachim-breitner.de/blog/786-A_Candid_explainer__Quirks
}

bool CandidDeserialize::deserialize_values_comptype(__uint128_t &B_offset,
                                                    const size_t index,
                                                    __uint128_t &numbytes,
                                                    std::string &parse_error) {
  // the datatype is an index into the typetable of the byte stream
  size_t type_table_index = m_args_datatypes[index];
  CandidTypeTable type_table = m_typetables[type_table_index];
  int opcode_found = type_table.get_opcode();

  int opcode_expected =
      std::visit([](auto &&c) { return c.get_datatype_opcode(); }, m_A[index]);

  if (opcode_found != opcode_expected) {
    std::string msg;
    msg.append("ERROR: wrong opcode found on wire.\n");
    msg.append("       Expecting opcode:" + std::to_string(opcode_expected) +
               "\n");
    msg.append("       Found opcode    :" + std::to_string(opcode_found));
    IC_API::trap(msg);
  }

  // Deserialize the values using the decoder of the datatype
  if (opcode_found == CandidOpcode().Record) {
    CandidTypeRecord *p_cRec =
        get_if<CandidTypeRecord>(type_table.get_candidType_ptr());
    // To fill the  C++ data values provided by caller
    CandidTypeRecord *p_expected = get_if<CandidTypeRecord>(&m_A[index]);

    if (const auto p_expected(get_if<CandidTypeRecord>(&m_A[index]));
        p_expected) {
      parse_error = "";
      __uint128_t B_offset_start = B_offset;
      if (p_cRec->decode_M(m_B, B_offset, parse_error, p_expected)) {
        std::string to_be_parsed = "Values";
        CandidDeserialize::trap_with_parse_error(B_offset_start, B_offset,
                                                 to_be_parsed, parse_error);
      }
    } else {
      IC_API::trap("All is NOT OK\n" + std::string(__func__));
    }

  } else {
    std::string msg;
    msg.append(
        "ERROR: NOT YET IMPLEMENTED TO PARSE VALUES FOR THIS COMPTYPE.\n");
    msg.append("       datatype = " + std::to_string(opcode_found) + "\n");
    msg.append("      " + std::string(__func__));
    IC_API::trap(msg);
  }

  return false;
}

bool CandidDeserialize::deserialize_values_primtype(__uint128_t &B_offset,
                                                    const size_t index,
                                                    __uint128_t &numbytes,
                                                    std::string &parse_error) {
  // The datatype is an Opcode
  int opcode_found = m_args_datatypes[index];
  int opcode_expected =
      std::visit([](auto &&c) { return c.get_datatype_opcode(); }, m_A[index]);

  if (opcode_found != opcode_expected) {
    // TODO:  IMPLEMENT CHECK ON COVARIANCE/CONTRAVARIANCE
    std::string msg;
    msg.append("ERROR: wrong opcode found on wire.\n");
    msg.append("       Expecting opcode:" + std::to_string(opcode_expected) +
               "\n");
    msg.append("       Found opcode    :" + std::to_string(opcode_found));
    IC_API::trap(msg);
  }

  // Deserialize the values using the decoder of the primtype
  parse_error = "";
  __uint128_t offset_start = B_offset;
  if (std::visit(
          [&](auto &&c) {
            return c.decode_M(m_B, B_offset, parse_error, nullptr);
          },
          m_A[index])) {
    std::string to_be_parsed = "Value for a primtype";
    CandidDeserialize::trap_with_parse_error(offset_start, B_offset,
                                             to_be_parsed, parse_error);
  }

  return false;
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
  msg.append("       byte offset:" +
             VecBytes::my_uint128_to_string(B_offset_start) + "\n");
  IC_API::trap(msg);
}