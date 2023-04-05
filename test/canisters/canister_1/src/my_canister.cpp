#include "my_canister.h"

#include <algorithm>
#include <string>

#include "compliance_test_construct.h"
#include "compliance_test_prim.h"
#include "ic0.h"
#include "ic_api.h"
#include "unit_test_candid.h"
#include "unit_test_ic_api.h"
#include "unit_test_vendors.h"

#include "vendors/nlohmann/json/json.hpp"

//----------------------------------------------------------------------------------
// Run all unit tests for vendor libraries
void test_vendors() {
  IC_API ic_api(false);
  int result = 0;
  result = std::max(result, unit_test_vendors());
  ic_api.to_wire(CandidTypeInt{result});
}

//----------------------------------------------------------------------------------
// Run all unit & compliance tests for candid
void test_candid() {
  IC_API ic_api(false);
  int result = 0;
  result = std::max(result, unit_test_candid());
  result = std::max(result, compliance_test_prim());
  result = std::max(result, compliance_test_construct());
  ic_api.to_wire(CandidTypeInt{result});
}

//----------------------------------------------------------------------------------
// Run all unit tests for ic_api
void test_ic_api() {
  IC_API ic_api(false);
  int result = unit_test_ic_api();
  ic_api.to_wire(CandidTypeInt{result});
}

//----------------------------------------------------------------------------------
// Run all roundtrip tests
void roundtrip_bool_true() {
  IC_API ic_api(false);
  bool in{false};
  ic_api.from_wire(CandidTypeBool{&in});
  if (!in) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeBool{in});
}

void roundtrip_bool_false() {
  IC_API ic_api(false);
  bool in{false};
  ic_api.from_wire(CandidTypeBool{&in});
  if (in) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeBool{in});
}

void roundtrip_nat_101() {
  IC_API ic_api(false);
  __uint128_t in{0};
  ic_api.from_wire(CandidTypeNat{&in});
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat{in});
}

void roundtrip_nat_1001() {
  IC_API ic_api(false);
  __uint128_t in{0};
  ic_api.from_wire(CandidTypeNat{&in});
  if (in != 1001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat{in});
}

void roundtrip_int_101() {
  IC_API ic_api(false);
  __int128_t in{0};
  ic_api.from_wire(CandidTypeInt{&in});
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt{in});
}

void roundtrip_int_1001() {
  IC_API ic_api(false);
  __int128_t in{0};
  ic_api.from_wire(CandidTypeInt{&in});
  if (in != 1001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt{in});
}

void roundtrip_int_101_neg() {
  IC_API ic_api(false);
  __int128_t in{0};
  ic_api.from_wire(CandidTypeInt{&in});
  if (in != -101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt{in});
}

void roundtrip_int_1001_neg() {
  IC_API ic_api(false);
  __int128_t in{0};
  ic_api.from_wire(CandidTypeInt{&in});
  if (in != -1001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt{in});
}

void roundtrip_nat8_101() {
  IC_API ic_api(false);
  uint8_t in{0};
  ic_api.from_wire(CandidTypeNat8{&in});
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat8{in});
}

void roundtrip_nat16_101() {
  IC_API ic_api(false);
  uint16_t in{0};
  ic_api.from_wire(CandidTypeNat16{&in});
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat16{in});
}

void roundtrip_nat16_1001() {
  IC_API ic_api(false);
  uint16_t in{0};
  ic_api.from_wire(CandidTypeNat16{&in});
  if (in != 1001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat16{in});
}

void roundtrip_nat32_101() {
  IC_API ic_api(false);
  uint32_t in{0};
  ic_api.from_wire(CandidTypeNat32{&in});
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat32{in});
}

void roundtrip_nat32_1_000_000_001() {
  IC_API ic_api(false);
  uint32_t in{0};
  ic_api.from_wire(CandidTypeNat32{&in});
  if (in != 1000000001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat32{in});
}

void roundtrip_nat64_101() {
  IC_API ic_api(false);

  uint64_t in{0};
  ic_api.from_wire(CandidTypeNat64{&in});
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat64{in});
}

void roundtrip_nat64_1_000_000_000_000_000_001() {
  IC_API ic_api(false);

  uint64_t in{0};
  ic_api.from_wire(CandidTypeNat64{&in});
  if (in != 1000000000000000001)
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat64{in});
}

void roundtrip_int8_101() {
  IC_API ic_api(false);
  int8_t in{0};
  ic_api.from_wire(CandidTypeInt8{&in});
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt8{in});
}

void roundtrip_int8_101_neg() {
  IC_API ic_api(false);
  int8_t in{0};
  ic_api.from_wire(CandidTypeInt8{&in});
  if (in != -101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt8{in});
}

void roundtrip_int16_101() {
  IC_API ic_api(false);
  int16_t in{0};
  ic_api.from_wire(CandidTypeInt16{&in});
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt16{in});
}

void roundtrip_int16_101_neg() {
  IC_API ic_api(false);
  int16_t in{0};
  ic_api.from_wire(CandidTypeInt16{&in});
  if (in != -101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt16{in});
}

void roundtrip_int16_1001() {
  IC_API ic_api(false);
  int16_t in{0};
  ic_api.from_wire(CandidTypeInt16{&in});
  if (in != 1001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt16{in});
}

void roundtrip_int16_1001_neg() {
  IC_API ic_api(false);
  int16_t in{0};
  ic_api.from_wire(CandidTypeInt16{&in});
  if (in != -1001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt16{in});
}

void roundtrip_int32_101() {
  IC_API ic_api(false);
  int32_t in{0};
  ic_api.from_wire(CandidTypeInt32{&in});
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt32{in});
}

void roundtrip_int32_101_neg() {
  IC_API ic_api(false);
  int32_t in{0};
  ic_api.from_wire(CandidTypeInt32{&in});
  if (in != -101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt32{in});
}

void roundtrip_int32_1_000_000_001() {
  IC_API ic_api(false);
  int32_t in{0};
  ic_api.from_wire(CandidTypeInt32{&in});
  if (in != 1000000001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt32{in});
}

void roundtrip_int32_1_000_000_001_neg() {
  IC_API ic_api(false);
  int32_t in{0};
  ic_api.from_wire(CandidTypeInt32{&in});
  if (in != -1000000001)
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt32{in});
}

void roundtrip_int64_101() {
  IC_API ic_api(false);
  int64_t in{0};
  ic_api.from_wire(CandidTypeInt64{&in});
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt64{in});
}

void roundtrip_int64_101_neg() {
  IC_API ic_api(false);
  int64_t in{0};
  ic_api.from_wire(CandidTypeInt64{&in});
  if (in != -101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt64{in});
}

void roundtrip_int64_1_000_000_000_000_000_001() {
  IC_API ic_api(false);
  int64_t in{0};
  ic_api.from_wire(CandidTypeInt64{&in});
  if (in != 1000000000000000001)
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt64{in});
}

void roundtrip_int64_1_000_000_000_000_000_001_neg() {
  IC_API ic_api(false);
  int64_t in{0};
  ic_api.from_wire(CandidTypeInt64{&in});
  if (in != -1000000000000000001)
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt64{in});
}

void roundtrip_text() {
  IC_API ic_api(false);
  std::string in{""};
  ic_api.from_wire(CandidTypeText{&in});
  if (in != "Hello C++ Canister")
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeText{in});
}

void roundtrip_text_to_json_to_text() {
  IC_API ic_api(false);

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
  IC_API ic_api(false);

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
  IC_API ic_api(false);
  ic_api.from_wire(CandidTypeReserved{});
  ic_api.to_wire(CandidTypeReserved{});
}

void roundtrip_float32() {
  IC_API ic_api(false);
  float in{0.0};
  ic_api.from_wire(CandidTypeFloat32{&in});
  if (!is_approximately_equal(in, (float)1001.1001))
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeFloat32{in});
}

void roundtrip_float32_neg() {
  IC_API ic_api(false);
  float in{0.0};
  ic_api.from_wire(CandidTypeFloat32{&in});
  if (!is_approximately_equal(in, (float)-1001.1001))
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeFloat32{in});
}

void roundtrip_float64() {
  IC_API ic_api(false);
  double in{0.0};
  ic_api.from_wire(CandidTypeFloat64{&in});
  if (!is_approximately_equal(in, (double)1001.1001))
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeFloat64{in});
}

void roundtrip_float64_neg() {
  IC_API ic_api(false);
  double in{0.0};
  ic_api.from_wire(CandidTypeFloat64{&in});
  if (!is_approximately_equal(in, (double)-1001.1001))
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeFloat64{in});
}

void roundtrip_principal() {
  IC_API ic_api(false);
  std::string in{""};
  ic_api.from_wire(CandidTypePrincipal{&in});
  if (in != "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae")
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypePrincipal{in});
}

void roundtrip_vec_nat16() {
  IC_API ic_api(false);
  std::vector<uint16_t> in;
  ic_api.from_wire(CandidTypeVecNat16{&in});
  if (in != std::vector<uint16_t>{101, 102, 103})
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));

  ic_api.to_wire(CandidTypeVecNat16{in});
}

void roundtrip_vec_all() {
  IC_API ic_api(false);

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

  std::vector<CandidType> args_in;
  args_in.push_back(CandidTypeVecBool{&in_bool});
  args_in.push_back(CandidTypeVecNat{&in_nat});
  args_in.push_back(CandidTypeVecNat8{&in_nat8});
  args_in.push_back(CandidTypeVecNat16{&in_nat16});
  args_in.push_back(CandidTypeVecNat32{&in_nat32});
  args_in.push_back(CandidTypeVecNat64{&in_nat64});
  args_in.push_back(CandidTypeVecInt{&in_int});
  args_in.push_back(CandidTypeVecInt8{&in_int8});
  args_in.push_back(CandidTypeVecInt16{&in_int16});
  args_in.push_back(CandidTypeVecInt32{&in_int32});
  args_in.push_back(CandidTypeVecInt64{&in_int64});
  args_in.push_back(CandidTypeVecFloat32{&in_float32});
  args_in.push_back(CandidTypeVecFloat64{&in_float64});
  args_in.push_back(CandidTypeVecText{&in_text});
  args_in.push_back(CandidTypeVecPrincipal{&in_principal});
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
  std::vector<CandidType> args_out;
  args_out.push_back(CandidTypeVecBool{in_bool});
  args_out.push_back(CandidTypeVecNat{in_nat});
  args_out.push_back(CandidTypeVecNat8{in_nat8});
  args_out.push_back(CandidTypeVecNat16{in_nat16});
  args_out.push_back(CandidTypeVecNat32{in_nat32});
  args_out.push_back(CandidTypeVecNat64{in_nat64});
  args_out.push_back(CandidTypeVecInt{in_int});
  args_out.push_back(CandidTypeVecInt8{in_int8});
  args_out.push_back(CandidTypeVecInt16{in_int16});
  args_out.push_back(CandidTypeVecInt32{in_int32});
  args_out.push_back(CandidTypeVecInt64{in_int64});
  args_out.push_back(CandidTypeVecFloat32{in_float32});
  args_out.push_back(CandidTypeVecFloat64{in_float64});
  args_out.push_back(CandidTypeVecText{in_text});
  args_out.push_back(CandidTypeVecPrincipal{in_principal});
  ic_api.to_wire(args_out);
}

void roundtrip_record() {
  IC_API ic_api(false);

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
  // std::vector<CandidType> args_in;
  // args_in.push_back(r_in);
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

//----------------------------------------------------------------------------------
// Trap tests  (The calling method will verify that these indeed trap)
void trap_wrong_number_of_args() {
  IC_API ic_api(false);
  bool in{false};
  ic_api.from_wire(CandidTypeBool{&in}); // this must trap
}
