#include "my_canister.h"

#include <algorithm>
#include <memory>
#include <string>
#include <variant>

#include "compliance_test_construct.h"
#include "compliance_test_prim.h"
#include "ic0.h"
#include "ic_api.h"
#include "unit_test_candid.h"
#include "unit_test_ic_api.h"
#include "unit_test_vendors.h"

#include <json/json.hpp>

//----------------------------------------------------------------------------------
// Run all unit tests for vendor libraries
void test_vendors() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  int result = 0;
  result = std::max(result, unit_test_vendors());
  ic_api.to_wire(CandidTypeInt{result});
}

//----------------------------------------------------------------------------------
// Run all unit & compliance tests for candid
void test_candid() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  int result = 0;
  result = std::max(result, unit_test_candid());
  result = std::max(result, compliance_test_prim());
  result = std::max(result, compliance_test_construct());
  ic_api.to_wire(CandidTypeInt{result});
}

//----------------------------------------------------------------------------------
// Run all unit tests for ic_api
void test_ic_api() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  int result = unit_test_ic_api();
  ic_api.to_wire(CandidTypeInt{result});
}

//----------------------------------------------------------------------------------
// Run all roundtrip tests
void roundtrip_deprecated_ic_api_constructor() {
  IC_API ic_api(false);
  ic_api.from_wire();
  ic_api.to_wire();
}

void roundtrip_no_arguments() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.from_wire();
  ic_api.to_wire();
}

void roundtrip_bool_true() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  bool in{false};
  ic_api.from_wire(CandidTypeBool{&in});
  if (!in) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeBool{in});
}

void roundtrip_bool_false() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  bool in{false};
  ic_api.from_wire(CandidTypeBool{&in});
  if (in) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeBool{in});
}

void roundtrip_float32() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  float in{0.0};
  ic_api.from_wire(CandidTypeFloat32{&in});
  if (!is_approximately_equal(in, (float)1001.1001))
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeFloat32{in});
}

void roundtrip_float32_neg() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  float in{0.0};
  ic_api.from_wire(CandidTypeFloat32{&in});
  if (!is_approximately_equal(in, (float)-1001.1001))
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeFloat32{in});
}

void roundtrip_float64() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  double in{0.0};
  ic_api.from_wire(CandidTypeFloat64{&in});
  if (!is_approximately_equal(in, (double)1001.1001))
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeFloat64{in});
}

void roundtrip_float64_neg() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  double in{0.0};
  ic_api.from_wire(CandidTypeFloat64{&in});
  if (!is_approximately_equal(in, (double)-1001.1001))
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeFloat64{in});
}

void roundtrip_nat_101() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  __uint128_t in{0};
  ic_api.from_wire(CandidTypeNat{&in});
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat{in});
}

void roundtrip_nat_1001() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  __uint128_t in{0};
  ic_api.from_wire(CandidTypeNat{&in});
  if (in != 1001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat{in});
}

void roundtrip_int_101() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  __int128_t in{0};
  ic_api.from_wire(CandidTypeInt{&in});
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt{in});
}

void roundtrip_int_1001() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  __int128_t in{0};
  ic_api.from_wire(CandidTypeInt{&in});
  if (in != 1001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt{in});
}

void roundtrip_int_101_neg() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  __int128_t in{0};
  ic_api.from_wire(CandidTypeInt{&in});
  if (in != -101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt{in});
}

void roundtrip_int_1001_neg() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  __int128_t in{0};
  ic_api.from_wire(CandidTypeInt{&in});
  if (in != -1001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt{in});
}

void roundtrip_nat8_101() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  uint8_t in{0};
  ic_api.from_wire(CandidTypeNat8{&in});
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat8{in});
}

void roundtrip_nat16_101() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  uint16_t in{0};
  ic_api.from_wire(CandidTypeNat16{&in});
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat16{in});
}

void roundtrip_nat16_1001() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  uint16_t in{0};
  ic_api.from_wire(CandidTypeNat16{&in});
  if (in != 1001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat16{in});
}

void roundtrip_nat32_101() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  uint32_t in{0};
  ic_api.from_wire(CandidTypeNat32{&in});
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat32{in});
}

void roundtrip_nat32_1_000_000_001() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  uint32_t in{0};
  ic_api.from_wire(CandidTypeNat32{&in});
  if (in != 1000000001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat32{in});
}

void roundtrip_nat64_101() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  uint64_t in{0};
  ic_api.from_wire(CandidTypeNat64{&in});
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat64{in});
}

void roundtrip_nat64_1_000_000_000_000_000_001() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  uint64_t in{0};
  ic_api.from_wire(CandidTypeNat64{&in});
  if (in != 1000000000000000001)
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat64{in});
}

void roundtrip_int8_101() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  int8_t in{0};
  ic_api.from_wire(CandidTypeInt8{&in});
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt8{in});
}

void roundtrip_int8_101_neg() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  int8_t in{0};
  ic_api.from_wire(CandidTypeInt8{&in});
  if (in != -101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt8{in});
}

void roundtrip_int16_101() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  int16_t in{0};
  ic_api.from_wire(CandidTypeInt16{&in});
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt16{in});
}

void roundtrip_int16_101_neg() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  int16_t in{0};
  ic_api.from_wire(CandidTypeInt16{&in});
  if (in != -101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt16{in});
}

void roundtrip_int16_1001() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  int16_t in{0};
  ic_api.from_wire(CandidTypeInt16{&in});
  if (in != 1001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt16{in});
}

void roundtrip_int16_1001_neg() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  int16_t in{0};
  ic_api.from_wire(CandidTypeInt16{&in});
  if (in != -1001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt16{in});
}

void roundtrip_int32_101() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  int32_t in{0};
  ic_api.from_wire(CandidTypeInt32{&in});
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt32{in});
}

void roundtrip_int32_101_neg() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  int32_t in{0};
  ic_api.from_wire(CandidTypeInt32{&in});
  if (in != -101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt32{in});
}

void roundtrip_int32_1_000_000_001() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  int32_t in{0};
  ic_api.from_wire(CandidTypeInt32{&in});
  if (in != 1000000001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt32{in});
}

void roundtrip_int32_1_000_000_001_neg() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  int32_t in{0};
  ic_api.from_wire(CandidTypeInt32{&in});
  if (in != -1000000001)
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt32{in});
}

void roundtrip_int64_101() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  int64_t in{0};
  ic_api.from_wire(CandidTypeInt64{&in});
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt64{in});
}

void roundtrip_int64_101_neg() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  int64_t in{0};
  ic_api.from_wire(CandidTypeInt64{&in});
  if (in != -101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt64{in});
}

void roundtrip_int64_1_000_000_000_000_000_001() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  int64_t in{0};
  ic_api.from_wire(CandidTypeInt64{&in});
  if (in != 1000000000000000001)
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt64{in});
}

void roundtrip_int64_1_000_000_000_000_000_001_neg() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  int64_t in{0};
  ic_api.from_wire(CandidTypeInt64{&in});
  if (in != -1000000000000000001)
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt64{in});
}

void roundtrip_text() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::string in{""};
  ic_api.from_wire(CandidTypeText{&in});
  if (in != "Hello C++ Canister")
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeText{in});
}

void roundtrip_text_to_json_to_text() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // Get the Candid Text from the wire
  std::string in{""};
  ic_api.from_wire(CandidTypeText{&in});

  // Convert it into a nlohmann/json object
  // See: https://json.nlohmann.me/api/basic_json/parse/
  nlohmann::json j = nlohmann::json::parse(in);

  // Example of how to extract the data from the json object
  // find an entry
  std::string project;
  if (j.contains("project")) {
    // there is an entry with key "project"
    project = j["project"];
  } else {
    IC_API::trap(std::string("\n\tapi_jsontext_to_jsontext: ERROR in JSON:\n\t "
                             "- missing key 'project' "));
  }

  int cpp_version;
  if (j.contains("C++")) {
    // there is an entry with key "C++"
    cpp_version = j["C++"];
  } else {
    IC_API::trap(std::string("\n\tapi_jsontext_to_jsontext: ERROR in JSON:\n\t "
                             "- missing key 'C++' "));
  }

  // Modify the JSON:
  j["C++"] = 20;
  j["project"] = "icpp";

  std::string s = j.dump();
  IC_API::debug_print("roundtrip_text_to_json_to_text: replying with text:");
  IC_API::debug_print(s);

  ic_api.to_wire(CandidTypeText{s});
}

void roundtrip_text_to_json_to_text_long_message() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // Get the Candid Text from the wire
  std::string in{""};
  ic_api.from_wire(CandidTypeText{&in});

  // Convert it into a nlohmann/json object
  // See: https://json.nlohmann.me/api/basic_json/parse/
  nlohmann::json j = nlohmann::json::parse(in);

  // just return the same thing, as a string
  std::string s = j.dump();

  IC_API::debug_print(
      "roundtrip_text_to_json_to_text_long_message: replying with text:");
  IC_API::debug_print(s);

  ic_api.to_wire(CandidTypeText{s});
}

void roundtrip_reserved() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.from_wire(CandidTypeReserved{});
  ic_api.to_wire(CandidTypeReserved{});
}

void roundtrip_principal() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::string in{""};
  ic_api.from_wire(CandidTypePrincipal{&in});
  if (in != "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae")
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypePrincipal{in});
}

void caller_is_anonymous() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  CandidTypePrincipal caller = ic_api.get_caller();
  bool is_anonymous = caller.is_anonymous();
  ic_api.to_wire(CandidTypeBool{is_anonymous});
}

void roundtrip_vec_nat16() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::vector<uint16_t> in;
  ic_api.from_wire(CandidTypeVecNat16{&in});
  if (in != std::vector<uint16_t>{101, 102, 103})
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));

  ic_api.to_wire(CandidTypeVecNat16{in});
}

void roundtrip_vec_record() {
  // This type used for headers in the http_request of the IC
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // ---------------------------------------------------------------------------
  // Get the data from the wire
  // vec { record {name = "H1N" : text; value = "H1V"}; record {name = "H2N" : text; value = "H2V"}; record {name = "H3N" : text; value = "H3V"}; }

  // Define vectors to store the data from the vec-of-records
  std::vector<std::string> names;
  std::vector<std::string> values;

  // Pass in a record-of-vecs using the correct keys, and icpp will know what to do ;-)
  CandidTypeRecord r_in;
  r_in.append("name", CandidTypeVecText{&names});
  r_in.append("value", CandidTypeVecText{&values});
  ic_api.from_wire(CandidTypeVecRecord{&r_in});

  // ---------------------------------------------------------------------------
  // Verify the data
  if (names.size() == 0)
    IC_API::trap("ASSERT ERROR no 'names' found - " + std::string(__func__));
  if (names.size() != values.size())
    IC_API::trap(
        "ASSERT ERROR different size found for 'names' & 'values' vectors - " +
        std::string(__func__));
  if (names[0] != "H1N")
    IC_API::trap("ASSERT ERROR names[0] - " + std::string(__func__));
  if (names[1] != "H2N")
    IC_API::trap("ASSERT ERROR names[1] - " + std::string(__func__));
  if (names[2] != "H3N")
    IC_API::trap("ASSERT ERROR names[2] - " + std::string(__func__));
  if (values[0] != "H1V")
    IC_API::trap("ASSERT ERROR values[0] - " + std::string(__func__));
  if (values[1] != "H2V")
    IC_API::trap("ASSERT ERROR values[1] - " + std::string(__func__));
  if (values[2] != "H3V")
    IC_API::trap("ASSERT ERROR values[2] - " + std::string(__func__));
  // ---------------------------------------------------------------------------

  CandidTypeRecord r_out;
  r_out.append("name", CandidTypeVecText{names});
  r_out.append("value", CandidTypeVecText{values});
  ic_api.to_wire(CandidTypeVecRecord{r_out});
}

void roundtrip_vec_all() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // ---------------------------------------------------------------------------
  // Get the data from the wire
  std::vector<bool> in_bool;
  std::vector<__uint128_t> in_nat;
  std::vector<uint8_t> in_nat8;
  std::vector<uint16_t> in_nat16;
  std::vector<uint32_t> in_nat32;
  std::vector<uint64_t> in_nat64;
  std::vector<__int128_t> in_int;
  std::vector<int8_t> in_int8;
  std::vector<int16_t> in_int16;
  std::vector<int32_t> in_int32;
  std::vector<int64_t> in_int64;
  std::vector<float> in_float32;
  std::vector<double> in_float64;
  std::vector<std::string> in_text;
  std::vector<std::string> in_principal;

  CandidArgs args_in;
  args_in.append(CandidTypeVecBool{&in_bool});
  args_in.append(CandidTypeVecNat{&in_nat});
  args_in.append(CandidTypeVecNat8{&in_nat8});
  args_in.append(CandidTypeVecNat16{&in_nat16});
  args_in.append(CandidTypeVecNat32{&in_nat32});
  args_in.append(CandidTypeVecNat64{&in_nat64});
  args_in.append(CandidTypeVecInt{&in_int});
  args_in.append(CandidTypeVecInt8{&in_int8});
  args_in.append(CandidTypeVecInt16{&in_int16});
  args_in.append(CandidTypeVecInt32{&in_int32});
  args_in.append(CandidTypeVecInt64{&in_int64});
  args_in.append(CandidTypeVecFloat32{&in_float32});
  args_in.append(CandidTypeVecFloat64{&in_float64});
  args_in.append(CandidTypeVecText{&in_text});
  args_in.append(CandidTypeVecPrincipal{&in_principal});
  ic_api.from_wire(args_in);

  // ---------------------------------------------------------------------------
  // Verify the data
  if (in_bool != std::vector<bool>{true, false})
    IC_API::trap("ASSERT ERROR - in_bool " + std::string(__func__));
  if (in_nat != std::vector<__uint128_t>{101, 102, 103})
    IC_API::trap("ASSERT ERROR - in_nat " + std::string(__func__));
  if (in_nat8 != std::vector<uint8_t>{101, 102, 103})
    IC_API::trap("ASSERT ERROR - in_nat8 " + std::string(__func__));
  if (in_nat16 != std::vector<uint16_t>{101, 102, 103})
    IC_API::trap("ASSERT ERROR - in_nat16 " + std::string(__func__));
  if (in_nat32 != std::vector<uint32_t>{101, 102, 103})
    IC_API::trap("ASSERT ERROR - in_nat32 " + std::string(__func__));
  if (in_nat64 != std::vector<uint64_t>{101, 102, 103})
    IC_API::trap("ASSERT ERROR - in_nat64 " + std::string(__func__));
  if (in_int != std::vector<__int128_t>{-101, -102, -103})
    IC_API::trap("ASSERT ERROR - in_int " + std::string(__func__));
  if (in_int8 != std::vector<int8_t>{-101, -102, -103})
    IC_API::trap("ASSERT ERROR - in_int8 " + std::string(__func__));
  if (in_int16 != std::vector<int16_t>{-101, -102, -103})
    IC_API::trap("ASSERT ERROR - in_int16 " + std::string(__func__));
  if (in_int32 != std::vector<int32_t>{-101, -102, -103})
    IC_API::trap("ASSERT ERROR - in_int32 " + std::string(__func__));
  if (in_int64 != std::vector<int64_t>{-101, -102, -103})
    IC_API::trap("ASSERT ERROR - in_int64 " + std::string(__func__));

  std::vector<float> in_float32_expected{(float)-1.01, (float)-1.02,
                                         (float)-1.03};
  std::vector<double> in_float64_expected{(double)-1.01, (double)-1.02,
                                          (double)-1.03};
  for (size_t i = 0; i < in_float32.size(); ++i) {
    if (!is_approximately_equal(in_float32[i], in_float32_expected[i]))
      IC_API::trap("ASSERT ERROR - in_float32 " + std::string(__func__));
    if (!is_approximately_equal(in_float64[i], in_float64_expected[i]))
      IC_API::trap("ASSERT ERROR - in_float64 " + std::string(__func__));
  }

  if (in_text !=
      std::vector<std::string>{"Hello 101", "Hello 102", "Hello 103"})
    IC_API::trap("ASSERT ERROR - in_text " + std::string(__func__));

  if (in_principal !=
      std::vector<std::string>{
          "2ibo7-dia", "w3gef-eqbai",
          "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae"})
    IC_API::trap("ASSERT ERROR - in_text " + std::string(__func__));

  // ---------------------------------------------------------------------------
  // Return the date to the wire
  CandidArgs args_out;
  args_out.append(CandidTypeVecBool{in_bool});
  args_out.append(CandidTypeVecNat{in_nat});
  args_out.append(CandidTypeVecNat8{in_nat8});
  args_out.append(CandidTypeVecNat16{in_nat16});
  args_out.append(CandidTypeVecNat32{in_nat32});
  args_out.append(CandidTypeVecNat64{in_nat64});
  args_out.append(CandidTypeVecInt{in_int});
  args_out.append(CandidTypeVecInt8{in_int8});
  args_out.append(CandidTypeVecInt16{in_int16});
  args_out.append(CandidTypeVecInt32{in_int32});
  args_out.append(CandidTypeVecInt64{in_int64});
  args_out.append(CandidTypeVecFloat32{in_float32});
  args_out.append(CandidTypeVecFloat64{in_float64});
  args_out.append(CandidTypeVecText{in_text});
  args_out.append(CandidTypeVecPrincipal{in_principal});
  ic_api.to_wire(args_out);
}

void roundtrip_opt_nat() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::optional<__uint128_t> in;
  ic_api.from_wire(CandidTypeOptNat{&in});
  if (!in.has_value() || in.value() != 101)
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));

  ic_api.to_wire(CandidTypeOptNat{in});
}

void roundtrip_opt_nat_no_value() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::optional<__uint128_t> in;
  ic_api.from_wire(CandidTypeOptNat{&in});
  if (in.has_value()) IC_API::trap("ASSERT ERROR - " + std::string(__func__));

  ic_api.to_wire(CandidTypeOptNat{in});
}

void roundtrip_opt_nat_nat() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::optional<__uint128_t> in1;
  __uint128_t in2;
  CandidArgs args_in;
  args_in.append(CandidTypeOptNat{&in1});
  args_in.append(CandidTypeNat{&in2});
  ic_api.from_wire(args_in);
  if (in2 != 102) IC_API::trap("ASSERT ERROR - " + std::string(__func__));

  ic_api.to_wire(CandidTypeNat{in2});
}

void roundtrip_opt_nat16() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::optional<uint16_t> in;
  ic_api.from_wire(CandidTypeOptNat16{&in});
  if (!in.has_value() || in.value() != 101)
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));

  ic_api.to_wire(CandidTypeOptNat16{in});
}

void roundtrip_opt_nat16_no_value() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::optional<uint16_t> in;
  ic_api.from_wire(CandidTypeOptNat16{&in});
  if (in.has_value()) IC_API::trap("ASSERT ERROR - " + std::string(__func__));

  ic_api.to_wire(CandidTypeOptNat16{in});
}

void roundtrip_opt_record() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  std::string s;
  uint16_t n{0};

  CandidTypeRecord r_in;
  r_in.append("field1", CandidTypeText{&s});
  r_in.append("field2", CandidTypeNat16{&n});

  bool has_value{false};
  ic_api.from_wire(CandidTypeOptRecord{&r_in, &has_value});

  // --
  if (!has_value || s != "hello" || n != 16) {
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  }

  // --
  CandidTypeRecord r_out;
  r_out.append("field1", CandidTypeText{s});
  r_out.append("field2", CandidTypeNat16{n});
  ic_api.to_wire(CandidTypeOptRecord{r_out});
}

void roundtrip_opt_record_no_value() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  std::string s;
  uint16_t n{0};

  CandidTypeRecord r_in;
  r_in.append("field1", CandidTypeText{&s});
  r_in.append("field2", CandidTypeNat16{&n});

  bool has_value{false};
  ic_api.from_wire(CandidTypeOptRecord{&r_in, &has_value});

  // --
  if (has_value) {
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  }

  // --
  ic_api.to_wire();
}

void roundtrip_opt_variant_ok() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  std::string label{""};
  std::string err{""};

  CandidTypeVariant v_in{&label};
  v_in.append("Ok", CandidTypeNull{});
  v_in.append("Err", CandidTypeText{&err});

  bool has_value{false};
  ic_api.from_wire(CandidTypeOptVariant{&v_in, &has_value});

  // --
  if (!has_value || label != "Ok") {
    IC_API::trap("ASSERT ERROR wrong label - " + std::string(__func__));
  }

  // --
  ic_api.to_wire(CandidTypeOptVariant{CandidTypeVariant{"Ok"}});
}

void roundtrip_opt_variant_ok_no_value() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  std::string label{""};
  std::string err{""};

  CandidTypeVariant v_in{&label};
  v_in.append("Ok", CandidTypeNull{});
  v_in.append("Err", CandidTypeText{&err});

  bool has_value{false};
  ic_api.from_wire(CandidTypeOptVariant{&v_in, &has_value});

  // --
  if (has_value) {
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  }

  // --
  ic_api.to_wire();
}

void roundtrip_opt_variant_err() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  std::string label{""};
  std::string ok{""};
  std::string err{""};

  CandidTypeVariant v_in{&label};
  v_in.append("Ok", CandidTypeText{&ok});
  v_in.append("Err", CandidTypeText{&err});

  bool has_value{false};
  ic_api.from_wire(CandidTypeOptVariant{&v_in, &has_value});

  // --
  if (!has_value || label != "Err") {
    IC_API::trap("ASSERT ERROR wrong label - " + std::string(__func__));
  }
  if (err != "Error")
    IC_API::trap("ASSERT ERROR wrong value - " + std::string(__func__));

  // --
  ic_api.to_wire(
      CandidTypeOptVariant{CandidTypeVariant{"Err", CandidTypeText{"Error"}}});
}

void roundtrip_opt_variant_err_no_value() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  std::string label{""};
  std::string ok{""};
  std::string err{""};

  CandidTypeVariant v_in{&label};
  v_in.append("Ok", CandidTypeText{&ok});
  v_in.append("Err", CandidTypeText{&err});

  bool has_value{false};
  ic_api.from_wire(CandidTypeOptVariant{&v_in, &has_value});

  // --
  if (has_value) {
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  }

  // --
  ic_api.to_wire();
}

void roundtrip_opt_record_variant() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  std::string s1;

  __int128_t iv;
  std::string labelv;
  CandidTypeVariant v_in{&labelv};
  v_in.append("fieldv", CandidTypeInt{&iv});

  std::string s3;

  CandidTypeRecord r_in;
  r_in.append("field1", CandidTypeText{&s1});
  r_in.append("field2", v_in);
  r_in.append("field3", CandidTypeText{&s3});

  bool has_value{false};
  ic_api.from_wire(CandidTypeOptRecord{&r_in, &has_value});

  // --
  if (!has_value || s1 != "hello" || iv != 16 || s3 != "bye") {
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  }

  // --
  CandidTypeVariant v_out{labelv};
  v_out.append("fieldv", CandidTypeInt{iv});

  CandidTypeRecord r_out;
  r_out.append("field1", CandidTypeText{s1});
  r_out.append("field2", v_out);
  r_out.append("field3", CandidTypeText{s3});
  ic_api.to_wire(CandidTypeOptRecord{r_out});
}

void roundtrip_opt_record_variant_no_value() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  std::string s1;

  __int128_t iv;
  std::string labelv;
  CandidTypeVariant v_in{&labelv};
  v_in.append("fieldv", CandidTypeInt{&iv});

  std::string s3;

  CandidTypeRecord r_in;
  r_in.append("field1", CandidTypeText{&s1});
  r_in.append("field2", v_in);
  r_in.append("field3", CandidTypeText{&s3});

  bool has_value{false};
  ic_api.from_wire(CandidTypeOptRecord{&r_in, &has_value});

  // --
  if (has_value) {
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  }

  // --
  ic_api.to_wire();
}

void roundtrip_opt_vec_nat16() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::vector<uint16_t> in;
  CandidTypeVecNat16 v_in{&in};

  bool has_value{false};
  ic_api.from_wire(CandidTypeOptVec{&v_in, &has_value});

  if (in != std::vector<uint16_t>{101, 102, 103})
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));

  CandidTypeVecNat16 v_out{in};
  ic_api.to_wire(CandidTypeOptVec{v_out});
}

void roundtrip_opt_vec_record() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // Define vectors to store the data from the vec-of-records
  std::vector<std::string> field1;
  std::vector<std::string> field2;

  // Pass in a record-of-vecs using the correct keys, and icpp will know what to do ;-)
  CandidTypeRecord r_in;
  r_in.append("field1", CandidTypeVecText{&field1});
  r_in.append("field2", CandidTypeVecText{&field2});
  CandidTypeVecRecord vr_in{&r_in};

  bool has_value{false};
  ic_api.from_wire(CandidTypeOptVec{&vr_in, &has_value});

  if (!has_value ||
      field1 != std::vector<std::string>{"hello1", "hello2", "hello3"} ||
      field2 != std::vector<std::string>{"bye1", "bye2", "bye3"})
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));

  // Pass in a record-of-vecs using the correct keys, and icpp will know what to do ;-)
  CandidTypeRecord r_out;
  r_out.append("field1", CandidTypeVecText{field1});
  r_out.append("field2", CandidTypeVecText{field2});
  CandidTypeVecRecord vr_out{r_out};
  ic_api.to_wire(CandidTypeOptVec{vr_out});
}

void roundtrip_opt_all() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // ---------------------------------------------------------------------------
  // Get the data from the wire
  std::optional<bool> in_bool_true;
  std::optional<bool> in_bool_false;
  std::optional<bool> in_bool_no_value;
  std::optional<__uint128_t> in_nat;
  std::optional<__uint128_t> in_nat_no_value;
  std::optional<uint8_t> in_nat8;
  std::optional<uint8_t> in_nat8_no_value;
  std::optional<uint16_t> in_nat16;
  std::optional<uint16_t> in_nat16_no_value;
  std::optional<uint32_t> in_nat32;
  std::optional<uint32_t> in_nat32_no_value;
  std::optional<uint64_t> in_nat64;
  std::optional<uint64_t> in_nat64_no_value;
  std::optional<__int128_t> in_int;
  std::optional<__int128_t> in_int_no_value;
  std::optional<int8_t> in_int8;
  std::optional<int8_t> in_int8_no_value;
  std::optional<int16_t> in_int16;
  std::optional<int16_t> in_int16_no_value;
  std::optional<int32_t> in_int32;
  std::optional<int32_t> in_int32_no_value;
  std::optional<int64_t> in_int64;
  std::optional<int64_t> in_int64_no_value;
  std::optional<float> in_float32;
  std::optional<float> in_float32_no_value;
  std::optional<double> in_float64;
  std::optional<double> in_float64_no_value;
  std::optional<std::string> in_text;
  std::optional<std::string> in_text_no_value;
  std::optional<std::string> in_principal;
  std::optional<std::string> in_principal_no_value;

  CandidArgs args_in;
  args_in.append(CandidTypeOptBool{&in_bool_true});
  args_in.append(CandidTypeOptBool{&in_bool_false});
  args_in.append(CandidTypeOptBool{&in_bool_no_value});
  args_in.append(CandidTypeOptNat{&in_nat});
  args_in.append(CandidTypeOptNat{&in_nat_no_value});
  args_in.append(CandidTypeOptNat8{&in_nat8});
  args_in.append(CandidTypeOptNat8{&in_nat8_no_value});
  args_in.append(CandidTypeOptNat16{&in_nat16});
  args_in.append(CandidTypeOptNat16{&in_nat16_no_value});
  args_in.append(CandidTypeOptNat32{&in_nat32});
  args_in.append(CandidTypeOptNat32{&in_nat32_no_value});
  args_in.append(CandidTypeOptNat64{&in_nat64});
  args_in.append(CandidTypeOptNat64{&in_nat64_no_value});
  args_in.append(CandidTypeOptInt{&in_int});
  args_in.append(CandidTypeOptInt{&in_int_no_value});
  args_in.append(CandidTypeOptInt8{&in_int8});
  args_in.append(CandidTypeOptInt8{&in_int8_no_value});
  args_in.append(CandidTypeOptInt16{&in_int16});
  args_in.append(CandidTypeOptInt16{&in_int16_no_value});
  args_in.append(CandidTypeOptInt32{&in_int32});
  args_in.append(CandidTypeOptInt32{&in_int32_no_value});
  args_in.append(CandidTypeOptInt64{&in_int64});
  args_in.append(CandidTypeOptInt64{&in_int64_no_value});
  args_in.append(CandidTypeOptFloat32{&in_float32});
  args_in.append(CandidTypeOptFloat32{&in_float32_no_value});
  args_in.append(CandidTypeOptFloat64{&in_float64});
  args_in.append(CandidTypeOptFloat64{&in_float64_no_value});
  args_in.append(CandidTypeOptText{&in_text});
  args_in.append(CandidTypeOptText{&in_text_no_value});
  args_in.append(CandidTypeOptPrincipal{&in_principal});
  args_in.append(CandidTypeOptPrincipal{&in_principal_no_value});
  ic_api.from_wire(args_in);

  // ---------------------------------------------------------------------------
  // Verify the data
  if (!in_bool_true.has_value() || !in_bool_true.value())
    IC_API::trap("ASSERT ERROR - in_bool_true " + std::string(__func__));
  if (!in_bool_false.has_value() || in_bool_false.value())
    IC_API::trap("ASSERT ERROR - in_bool_false " + std::string(__func__));
  if (in_bool_no_value.has_value())
    IC_API::trap("ASSERT ERROR - in_bool_no_value " + std::string(__func__));

  if (!in_nat.has_value() || in_nat.value() != 101)
    IC_API::trap("ASSERT ERROR - in_nat " + std::string(__func__));
  if (in_nat_no_value.has_value())
    IC_API::trap("ASSERT ERROR - in_nat_no_value " + std::string(__func__));
  if (!in_nat8.has_value() || in_nat8.value() != 102)
    IC_API::trap("ASSERT ERROR - in_nat8 " + std::string(__func__));
  if (in_nat8_no_value.has_value())
    IC_API::trap("ASSERT ERROR - in_nat8_no_value " + std::string(__func__));
  if (!in_nat16.has_value() || in_nat16.value() != 103)
    IC_API::trap("ASSERT ERROR - in_nat16 " + std::string(__func__));
  if (in_nat16_no_value.has_value())
    IC_API::trap("ASSERT ERROR - in_nat16_no_value " + std::string(__func__));
  if (!in_nat32.has_value() || in_nat32.value() != 104)
    IC_API::trap("ASSERT ERROR - in_nat32 " + std::string(__func__));
  if (in_nat32_no_value.has_value())
    IC_API::trap("ASSERT ERROR - in_nat32_no_value " + std::string(__func__));
  if (!in_nat64.has_value() || in_nat64.value() != 105)
    IC_API::trap("ASSERT ERROR - in_nat64 " + std::string(__func__));
  if (in_nat64_no_value.has_value())
    IC_API::trap("ASSERT ERROR - in_nat64_no_value " + std::string(__func__));

  if (!in_int.has_value() || in_int.value() != -101)
    IC_API::trap("ASSERT ERROR - in_int " + std::string(__func__));
  if (in_int_no_value.has_value())
    IC_API::trap("ASSERT ERROR - in_int_no_value " + std::string(__func__));
  if (!in_int8.has_value() || in_int8.value() != -102)
    IC_API::trap("ASSERT ERROR - in_int8 " + std::string(__func__));
  if (in_int8_no_value.has_value())
    IC_API::trap("ASSERT ERROR - in_int8_no_value " + std::string(__func__));
  if (!in_int16.has_value() || in_int16.value() != -103)
    IC_API::trap("ASSERT ERROR - in_int16 " + std::string(__func__));
  if (in_int16_no_value.has_value())
    IC_API::trap("ASSERT ERROR - in_int16_no_value " + std::string(__func__));
  if (!in_int32.has_value() || in_int32.value() != -104)
    IC_API::trap("ASSERT ERROR - in_int32 " + std::string(__func__));
  if (in_int32_no_value.has_value())
    IC_API::trap("ASSERT ERROR - in_int32_no_value " + std::string(__func__));
  if (!in_int64.has_value() || in_int64.value() != -105)
    IC_API::trap("ASSERT ERROR - in_int64 " + std::string(__func__));
  if (in_int64_no_value.has_value())
    IC_API::trap("ASSERT ERROR - in_int64_no_value " + std::string(__func__));

  if (!in_float32.has_value() ||
      !is_approximately_equal(in_float32.value(), (float)-1.01))
    IC_API::trap("ASSERT ERROR - in_float32 " + std::string(__func__));
  if (in_float32_no_value.has_value())
    IC_API::trap("ASSERT ERROR - in_float32_no_value " + std::string(__func__));

  if (!in_float64.has_value() ||
      !is_approximately_equal(in_float64.value(), (double)-1.02))
    IC_API::trap("ASSERT ERROR - in_float64 " + std::string(__func__));
  if (in_float64_no_value.has_value())
    IC_API::trap("ASSERT ERROR - in_float64_no_value " + std::string(__func__));

  if (!in_text.has_value() || in_text.value() != "Hello 101")
    IC_API::trap("ASSERT ERROR - in_text " + std::string(__func__));
  if (in_text_no_value.has_value())
    IC_API::trap("ASSERT ERROR - in_text_no_value " + std::string(__func__));

  if (!in_principal.has_value() ||
      in_principal.value() !=
          "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae")
    IC_API::trap("ASSERT ERROR - in_principal " + std::string(__func__));
  if (in_principal_no_value.has_value())
    IC_API::trap("ASSERT ERROR - in_principal_no_value " +
                 std::string(__func__));

  // ---------------------------------------------------------------------------
  // Return the date to the wire
  CandidArgs args_out;
  args_out.append(CandidTypeOptBool{in_bool_true});
  args_out.append(CandidTypeOptBool{in_bool_false});
  args_out.append(CandidTypeOptBool{in_bool_no_value});
  args_out.append(CandidTypeOptNat{in_nat});
  args_out.append(CandidTypeOptNat{in_nat_no_value});
  args_out.append(CandidTypeOptNat8{in_nat8});
  args_out.append(CandidTypeOptNat8{in_nat8_no_value});
  args_out.append(CandidTypeOptNat16{in_nat16});
  args_out.append(CandidTypeOptNat16{in_nat16_no_value});
  args_out.append(CandidTypeOptNat32{in_nat32});
  args_out.append(CandidTypeOptNat32{in_nat32_no_value});
  args_out.append(CandidTypeOptNat64{in_nat64});
  args_out.append(CandidTypeOptNat64{in_nat64_no_value});
  args_out.append(CandidTypeOptInt{in_int});
  args_out.append(CandidTypeOptInt{in_int_no_value});
  args_out.append(CandidTypeOptInt8{in_int8});
  args_out.append(CandidTypeOptInt8{in_int8_no_value});
  args_out.append(CandidTypeOptInt16{in_int16});
  args_out.append(CandidTypeOptInt16{in_int16_no_value});
  args_out.append(CandidTypeOptInt32{in_int32});
  args_out.append(CandidTypeOptInt32{in_int32_no_value});
  args_out.append(CandidTypeOptInt64{in_int64});
  args_out.append(CandidTypeOptInt64{in_int64_no_value});
  args_out.append(CandidTypeOptFloat32{in_float32});
  args_out.append(CandidTypeOptFloat32{in_float32_no_value});
  args_out.append(CandidTypeOptFloat64{in_float64});
  args_out.append(CandidTypeOptFloat64{in_float64_no_value});
  args_out.append(CandidTypeOptText{in_text});
  args_out.append(CandidTypeOptText{in_text_no_value});
  args_out.append(CandidTypeOptPrincipal{in_principal});
  args_out.append(CandidTypeOptPrincipal{in_principal_no_value});
  ic_api.to_wire(args_out);
}

void roundtrip_record() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // ---------------------------------------------------------------------------
  // Get the data from the wire

  std::string name{""};
  double secret_x{0.0};
  __int128_t secret_i{0};

  CandidTypeRecord r_in;
  r_in.append("name", CandidTypeText{&name});
  r_in.append("secret float64", CandidTypeFloat64{&secret_x});
  r_in.append("secret int", CandidTypeInt{&secret_i});
  // r_in.append("secret int", CandidTypeText{&name});// TO TEST A TEMPORARY ERROR
  ic_api.from_wire(r_in);
  //
  // CandidArgs args_in;
  // args_in.append(r_in);
  // ic_api.from_wire(args_in);

  // ---------------------------------------------------------------------------
  // Verify the data
  if (name != "C++ Developer")
    IC_API::trap("ASSERT ERROR string - " + std::string(__func__));
  if (!is_approximately_equal(secret_x, (double)0.01))
    IC_API::trap("ASSERT ERROR double - " + std::string(__func__));
  if (secret_i != 11)
    IC_API::trap("ASSERT ERROR int - " + std::string(__func__));
  // ---------------------------------------------------------------------------

  std::string greeting = "Hello " + name + "!";
  std::string message = "Your secret numbers are:";

  CandidTypeRecord r_out;
  r_out.append("greeting", CandidTypeText{greeting});
  r_out.append("message", CandidTypeText{message});
  r_out.append("secret float64", CandidTypeFloat64{secret_x});
  r_out.append("secret int", CandidTypeInt{secret_i});
  ic_api.to_wire(r_out);
}

void roundtrip_record_opt() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // ---------------------------------------------------------------------------
  // Get the data from the wire
  uint8_t test_version{0};

  // Reverse order on purpose, to test sorting of field labels
  std::optional<std::string> field3;
  std::optional<uint16_t> field1;

  CandidTypeRecord r_in;
  r_in.append("field3", CandidTypeOptText{&field3});
  r_in.append("field1", CandidTypeOptNat16{&field1});

  CandidArgs args_in;
  args_in.append(CandidTypeNat8{&test_version});
  args_in.append(r_in);
  ic_api.from_wire(args_in);

  // ---------------------------------------------------------------------------
  // Verify the data
  bool assert_error = false;
  switch (test_version) {
  case 1: // Both fields have a value
    if ((!field1.has_value() || field1.value() != 16) ||
        (!field3.has_value() || field3.value() != "hello"))
      assert_error = true;
    break;
  case 2: // Both fields have no value
    if (field3.has_value() || field1.has_value()) assert_error = true;
    break;
  case 3: // Only field 1 has a value
    if ((!field1.has_value() || field1.value() != 16) || (field3.has_value()))
      assert_error = true;
    break;
  case 4: // Only field 3 has a value
    if ((field1.has_value()) ||
        (!field3.has_value() || field3.value() != "hello"))
      assert_error = true;
    break;
  default:
    IC_API::trap("ASSERT ERROR - Unknown test_version = " +
                 std::to_string(test_version) + " - " + std::string(__func__));
    break;
  }
  if (assert_error) {
    IC_API::trap("ASSERT ERROR. \n- test_version = " +
                 std::to_string(test_version) + "\n- " + std::string(__func__));
  }
  // ---------------------------------------------------------------------------
  // We always return the same

  CandidTypeRecord r_out;
  r_out.append("field3", CandidTypeOptText{"hello"});
  r_out.append("field1", CandidTypeOptNat16{16});
  ic_api.to_wire(r_out);
}

void roundtrip_record_opt_variant() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // ---------------------------------------------------------------------------
  // Get the data from the wire
  uint8_t test_version{0};

  std::string label1{""};
  std::string err1{""};
  CandidTypeVariant field1{&label1};
  field1.append("Ok", CandidTypeNull{});
  field1.append("Err", CandidTypeText{&err1});
  bool has_value1{false};

  std::string label3{""};
  std::string ok3{""};
  std::string err3{""};
  CandidTypeVariant field3{&label3};
  field3.append("Ok", CandidTypeText{&ok3});
  field3.append("Err", CandidTypeText{&err3});
  bool has_value3{false};

  // Reverse order on purpose, to test sorting of field labels
  CandidTypeRecord r_in;
  r_in.append("field3", CandidTypeOptVariant{&field3, &has_value3});
  r_in.append("field1", CandidTypeOptVariant{&field1, &has_value1});

  CandidArgs args_in;
  args_in.append(CandidTypeNat8{&test_version});
  args_in.append(r_in);
  ic_api.from_wire(args_in);

  // ---------------------------------------------------------------------------
  // Verify the data
  bool assert_error = false;
  switch (test_version) {
  case 1: // Both fields have a value
    if ((!has_value1 || label1 != "Ok") ||
        (!has_value3 || label3 != "Err" || err3 != "Error"))
      assert_error = true;
    break;
  case 2: // Both fields have no value
    if (has_value1 || has_value3) assert_error = true;
    break;
  case 3: // Only field 1 has a value
    if ((!has_value1 || label1 != "Ok") || (has_value3)) assert_error = true;
    break;
  case 4: // Only field 3 has a value
    if ((has_value1) || (!has_value3 || label3 != "Err" || err3 != "Error"))
      assert_error = true;
    break;
  default:
    IC_API::trap("ASSERT ERROR - Unknown test_version = " +
                 std::to_string(test_version) + " - " + std::string(__func__));
    break;
  }
  if (assert_error) {
    IC_API::trap("ASSERT ERROR. \n- test_version = " +
                 std::to_string(test_version) + "\n- " + std::string(__func__));
  }
  // ---------------------------------------------------------------------------
  // We always return the same

  CandidTypeRecord r_out;
  r_out.append("field3", CandidTypeOptVariant{CandidTypeVariant{
                             "Err", CandidTypeText{"Error"}}});
  r_out.append("field1", CandidTypeOptVariant{CandidTypeVariant{"Ok"}});
  ic_api.to_wire(r_out);
}

// test for nested records, resulting in nested type tables
void roundtrip_record_record_record() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // ---------------------------------------------------------------------------
  // Get the data from the wire
  // '(record {"key1"        = record { "key2"        = record {"key3"         = "value1" : text;}}})'
  // '(record {1_191_633_330 = record { 1_191_633_331 = record { 1_191_633_332 = "value1" };

  std::string v;

  // Order matters!
  CandidTypeRecord r_in_3;
  r_in_3.append("key3", CandidTypeText{&v});

  CandidTypeRecord r_in_2;
  r_in_2.append("key2", r_in_3);

  CandidTypeRecord r_in_1;
  r_in_1.append("key1", r_in_2);

  ic_api.from_wire(r_in_1);

  // ---------------------------------------------------------------------------
  // Verify the data
  if (v != "value1") IC_API::trap("ASSERT ERROR v - " + std::string(__func__));

  // Order matters!
  CandidTypeRecord r_out_3;
  r_out_3.append("key3", CandidTypeText{v});

  CandidTypeRecord r_out_2;
  r_out_2.append("key2", r_out_3);

  CandidTypeRecord r_out_1;
  r_out_1.append("key1", r_out_2);

  ic_api.to_wire(r_out_1);
}

void roundtrip_record_vec_text() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // ---------------------------------------------------------------------------
  // Get the data from the wire
  // '(record {"names" = vec { "H1N" : text; "H2N" : text}; "values" = vec { "H1V" : text; "H2V" : text};})'

  std::vector<std::string> names;
  std::vector<std::string> values;

  CandidTypeRecord r_in;
  r_in.append("names", CandidTypeVecText{&names});
  r_in.append("values", CandidTypeVecText{&values});
  ic_api.from_wire(r_in);

  // ---------------------------------------------------------------------------
  // Verify the data
  if (names.size() == 0)
    IC_API::trap("ASSERT ERROR no 'names' found - " + std::string(__func__));
  if (names.size() != values.size())
    IC_API::trap(
        "ASSERT ERROR different size found for 'names' & 'values' vectors - " +
        std::string(__func__));
  if (names[0] != "H1N")
    IC_API::trap("ASSERT ERROR names[0] - " + std::string(__func__));
  if (names[1] != "H2N")
    IC_API::trap("ASSERT ERROR names[1] - " + std::string(__func__));
  if (values[0] != "H1V")
    IC_API::trap("ASSERT ERROR values[0] - " + std::string(__func__));
  if (values[1] != "H2V")
    IC_API::trap("ASSERT ERROR values[1] - " + std::string(__func__));
  // ---------------------------------------------------------------------------

  CandidTypeRecord r_out;
  r_out.append("names", CandidTypeVecText{names});
  r_out.append("values", CandidTypeVecText{values});
  ic_api.to_wire(r_out);
}

void roundtrip_record_vec_record() {
  // See also roundtrip_vec_record
  // This represents the headers in the http_request of the IC
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // ---------------------------------------------------------------------------
  // Get the data from the wire
  // '(record { headers = vec { record {name = "H1N" : text; value = "H1V"}; record {name = "H2N" : text; value = "H2V"}; record {name = "H3N" : text; value = "H3V"}; } })'

  // Define vectors to store the data from the vec-of-records
  std::vector<std::string> names;
  std::vector<std::string> values;

  // Pass in a record-of-vecs using the correct keys, and icpp will know what to do ;-)
  CandidTypeRecord header_fields_in;
  header_fields_in.append("name", CandidTypeVecText{&names});
  header_fields_in.append("value", CandidTypeVecText{&values});
  CandidTypeVecRecord headers_in{&header_fields_in};

  // The HttpRequest
  CandidTypeRecord http_in;
  http_in.append("headers", headers_in);

  ic_api.from_wire(http_in);

  // ---------------------------------------------------------------------------
  // Verify the data
  if (names.size() == 0)
    IC_API::trap("ASSERT ERROR no 'names' found - " + std::string(__func__));
  if (names.size() != values.size())
    IC_API::trap(
        "ASSERT ERROR different size found for 'names' & 'values' vectors - " +
        std::string(__func__));
  if (names[0] != "H1N")
    IC_API::trap("ASSERT ERROR names[0] - " + std::string(__func__));
  if (names[1] != "H2N")
    IC_API::trap("ASSERT ERROR names[1] - " + std::string(__func__));
  if (names[2] != "H3N")
    IC_API::trap("ASSERT ERROR names[2] - " + std::string(__func__));
  if (values[0] != "H1V")
    IC_API::trap("ASSERT ERROR values[0] - " + std::string(__func__));
  if (values[1] != "H2V")
    IC_API::trap("ASSERT ERROR values[1] - " + std::string(__func__));
  if (values[2] != "H3V")
    IC_API::trap("ASSERT ERROR values[2] - " + std::string(__func__));
  // ---------------------------------------------------------------------------

  CandidTypeRecord header_fields_out;
  header_fields_out.append("name", CandidTypeVecText{&names});
  header_fields_out.append("value", CandidTypeVecText{&values});
  CandidTypeVecRecord headers_out{header_fields_out};

  // The HttpResponse
  CandidTypeRecord http_out;
  http_out.append("headers", headers_out);

  ic_api.to_wire(http_out);
}

void roundtrip_variant_ok() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  // ---------------------------------------------------------------------------
  // Get the data from the wire
  std::string label{""};
  std::string err{""};

  CandidTypeVariant v_in{&label};
  v_in.append("Ok", CandidTypeNull{});
  v_in.append("Err", CandidTypeText{&err});
  ic_api.from_wire(v_in);

  // Verify the data
  if (label != "Ok")
    IC_API::trap("ASSERT ERROR wrong label - " + std::string(__func__));

  // ---------------------------------------------------------------------------
  // Send it back
  // CandidTypeVariant v_out{"Ok"};
  // v_out.append("Ok", CandidTypeNull{});
  // v_out.append("Err", CandidTypeText{"Error"});
  // ic_api.to_wire(v_out);
  ic_api.to_wire(CandidTypeVariant{"Ok"});
}

void roundtrip_variant_err() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  // ---------------------------------------------------------------------------
  // Get the data from the wire
  std::string label{""};
  std::string ok{""};
  std::string err{""};

  CandidTypeVariant v_in{&label};
  v_in.append("Ok", CandidTypeText{&ok});
  v_in.append("Err", CandidTypeText{&err});
  ic_api.from_wire(v_in);

  // Verify the data
  if (label != "Err")
    IC_API::trap("ASSERT ERROR wrong label - " + std::string(__func__));
  if (err != "Error")
    IC_API::trap("ASSERT ERROR wrong value - " + std::string(__func__));

  // ---------------------------------------------------------------------------
  // Send it back
  // CandidTypeVariant v_out{"Err"};
  // v_out.append("Ok", CandidTypeText{});
  // v_out.append("Err", CandidTypeText{"Error"});
  // ic_api.to_wire(v_out);
  ic_api.to_wire(CandidTypeVariant{"Err", CandidTypeText{"Error"}});
}

void roundtrip_variant() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  // ---------------------------------------------------------------------------
  // Get the data from the wire

  //   Result
  // '(variant { Ok })'
  std::string label_a{""};
  uint16_t err_a{0};
  CandidTypeVariant v_in_a{&label_a};
  v_in_a.append("Ok", CandidTypeNull{});
  v_in_a.append("Err", CandidTypeNat16{&err_a});

  //   Result
  // '(variant { Err = 404 : nat16})'
  std::string label_b{""};
  uint16_t err_b{0};
  CandidTypeVariant v_in_b{&label_b};
  v_in_b.append("Ok", CandidTypeNull{});
  v_in_b.append("Err", CandidTypeNat16{&err_b});

  //   Result_1
  // '(variant { Ok = "All good" : text})'
  std::string label_1a{""};
  std::string ok_1a{""};
  uint16_t err_1a{0};
  CandidTypeVariant v_in_1a{&label_1a};
  v_in_1a.append("Ok", CandidTypeText{&ok_1a});
  v_in_1a.append("Err", CandidTypeNat16{&err_1a});

  //   Result_1
  // '(variant { Err = 500 : nat16})'
  std::string label_1b{""};
  std::string ok_1b{""};
  uint16_t err_1b{0};
  CandidTypeVariant v_in_1b{&label_1b};
  v_in_1b.append("Ok", CandidTypeText{&ok_1b});
  v_in_1b.append("Err", CandidTypeNat16{&err_1b});

  //   Result_2
  // '(variant { Ok = "All good" : text})'
  std::string label_2a{""};
  std::string ok_2a{""};
  std::string err_2a{""};
  CandidTypeVariant v_in_2a{&label_2a};
  v_in_2a.append("Ok", CandidTypeText{&ok_2a});
  v_in_2a.append("Err", CandidTypeText{&err_2a});

  //   Result_2
  // '(variant { Err = "Error" : text})'
  std::string label_2b{""};
  std::string ok_2b{""};
  std::string err_2b{""};
  CandidTypeVariant v_in_2b{&label_2b};
  v_in_2b.append("Ok", CandidTypeText{&ok_2b});
  v_in_2b.append("Err", CandidTypeText{&err_2b});

  CandidArgs args_in;
  args_in.append(v_in_a);
  args_in.append(v_in_b);
  args_in.append(v_in_1a);
  args_in.append(v_in_1b);
  args_in.append(v_in_2a);
  args_in.append(v_in_2b);
  ic_api.from_wire(args_in);

  // Verify the data
  if (label_a != "Ok")
    IC_API::trap("ASSERT ERROR wrong label_a - " + std::string(__func__));

  if (label_b != "Err")
    IC_API::trap("ASSERT ERROR wrong label_b - " + std::string(__func__));
  if (err_b != 404)
    IC_API::trap("ASSERT ERROR wrong value err_b - " + std::string(__func__));

  if (label_1a != "Ok")
    IC_API::trap("ASSERT ERROR wrong label_1a - " + std::string(__func__));
  if (ok_1a != "All good")
    IC_API::trap("ASSERT ERROR wrong value ok_1a - " + std::string(__func__));

  if (label_1b != "Err")
    IC_API::trap("ASSERT ERROR wrong label_1b - " + std::string(__func__));
  if (err_1b != 500)
    IC_API::trap("ASSERT ERROR wrong value err_1b - " + std::string(__func__));

  if (label_2a != "Ok")
    IC_API::trap("ASSERT ERROR wrong label_2a - " + std::string(__func__));
  if (ok_2a != "All good")
    IC_API::trap("ASSERT ERROR wrong value ok_2a - " + std::string(__func__));

  if (label_2b != "Err")
    IC_API::trap("ASSERT ERROR wrong label_2b - " + std::string(__func__));
  if (err_2b != "Error")
    IC_API::trap("ASSERT ERROR wrong value err_2b - " + std::string(__func__));

  // ---------------------------------------------------------------------------
  // Send it back
  CandidTypeVariant v_out_a{label_a};
  v_out_a.append("Ok", CandidTypeNull{});
  v_out_a.append("Err", CandidTypeNat16{err_a});

  CandidTypeVariant v_out_b{label_b};
  v_out_b.append("Ok", CandidTypeNull{});
  v_out_b.append("Err", CandidTypeNat16{err_b});

  CandidTypeVariant v_out_1a{label_1a};
  v_out_1a.append("Ok", CandidTypeText{ok_1a});
  v_out_1a.append("Err", CandidTypeNat16{err_1a});

  CandidTypeVariant v_out_1b{label_1b};
  v_out_1b.append("Ok", CandidTypeText{ok_1b});
  v_out_1b.append("Err", CandidTypeNat16{err_1b});

  CandidTypeVariant v_out_2a{label_2a};
  v_out_2a.append("Ok", CandidTypeText{ok_2a});
  v_out_2a.append("Err", CandidTypeText{err_2a});

  CandidTypeVariant v_out_2b{label_2b};
  v_out_2b.append("Ok", CandidTypeText{ok_2b});
  v_out_2b.append("Err", CandidTypeText{err_2b});

  CandidArgs args_out;
  args_out.append(v_out_a);
  args_out.append(v_out_b);
  args_out.append(v_out_1a);
  args_out.append(v_out_1b);
  args_out.append(v_out_2a);
  args_out.append(v_out_2b);
  ic_api.to_wire(args_out);
}

void roundtrip_variant_opt() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // ---------------------------------------------------------------------------
  // Get the data from the wire
  uint8_t test_version{0};

  // always expecting '(variant { Err = opt ("Err" : text})'
  std::string label{""};
  std::optional<std::string> err;
  CandidTypeVariant v_in{&label};
  v_in.append("Ok", CandidTypeOptNull{});
  v_in.append("Err", CandidTypeOptText{&err});

  CandidArgs args_in;
  args_in.append(CandidTypeNat8{&test_version});
  args_in.append(v_in);
  ic_api.from_wire(args_in);

  // ---------------------------------------------------------------------------
  // Verify the data
  bool assert_error = false;
  switch (test_version) {
  case 1: // Field is provided with a value
    if (!err.has_value() || err.value() != "Error") assert_error = true;
    break;
  case 2: // Field has no value
    if (err.has_value()) assert_error = true;
    break;
  default:
    IC_API::trap("ASSERT ERROR - Unknown test_version = " +
                 std::to_string(test_version) + " - " + std::string(__func__));
    break;
  }
  if (assert_error) {
    IC_API::trap("ASSERT ERROR. \n- test_version = " +
                 std::to_string(test_version) + "\n- " + std::string(__func__));
  }
  // ---------------------------------------------------------------------------
  // We always return the same

  label = "Err";
  CandidTypeVariant v_out{label};
  v_out.append("Err", CandidTypeOptText{"Error"});
  ic_api.to_wire(v_out);
}

//----------------------------------------------------------------------------------
// Trap tests  (The calling method will verify that these indeed trap)
void trap_wrong_number_of_args() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  bool in{false};
  ic_api.from_wire(CandidTypeBool{&in}); // this must trap
}

void trap_multiple_calls_from_wire() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  bool in{false};
  ic_api.from_wire(CandidTypeBool{&in});
  ic_api.from_wire(CandidTypeBool{&in}); // this must trap
}

void trap_multiple_calls_to_wire() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  bool in{false};
  ic_api.from_wire(CandidTypeBool{&in});

  ic_api.to_wire(CandidTypeBool{in});
  ic_api.to_wire(CandidTypeBool{in}); // this must trap
}
