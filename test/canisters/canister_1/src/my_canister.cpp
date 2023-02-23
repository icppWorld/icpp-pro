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

void roundtrip_bool_true() {
  IC_API ic_api(false);
  bool in{false};
  ic_api.from_wire(CandidTypeBool(&in));
  if (!in) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeBool(in));
}

void roundtrip_bool_false() {
  IC_API ic_api(false);
  bool in{false};
  ic_api.from_wire(CandidTypeBool(&in));
  if (in) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeBool(in));
}

void roundtrip_nat_101() {
  IC_API ic_api(false);
  __uint128_t in{0};
  ic_api.from_wire(CandidTypeNat(&in));
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat(in));
}

void roundtrip_nat_1001() {
  IC_API ic_api(false);
  __uint128_t in{0};
  ic_api.from_wire(CandidTypeNat(&in));
  if (in != 1001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat(in));
}

void roundtrip_int_101() {
  IC_API ic_api(false);
  __int128_t in{0};
  ic_api.from_wire(CandidTypeInt(&in));
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt(in));
}

void roundtrip_int_1001() {
  IC_API ic_api(false);
  __int128_t in{0};
  ic_api.from_wire(CandidTypeInt(&in));
  if (in != 1001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt(in));
}

void roundtrip_int_101_neg() {
  IC_API ic_api(false);
  __int128_t in{0};
  ic_api.from_wire(CandidTypeInt(&in));
  if (in != -101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt(in));
}

void roundtrip_int_1001_neg() {
  IC_API ic_api(false);
  __int128_t in{0};
  ic_api.from_wire(CandidTypeInt(&in));
  if (in != -1001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt(in));
}

void roundtrip_nat8_101() {
  IC_API ic_api(false);
  uint8_t in{0};
  ic_api.from_wire(CandidTypeNat8(&in));
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat8(in));
}

void roundtrip_nat16_101() {
  IC_API ic_api(false);
  uint16_t in{0};
  ic_api.from_wire(CandidTypeNat16(&in));
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat16(in));
}

void roundtrip_nat16_1001() {
  IC_API ic_api(false);
  uint16_t in{0};
  ic_api.from_wire(CandidTypeNat16(&in));
  if (in != 1001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat16(in));
}

void roundtrip_nat32_101() {
  IC_API ic_api(false);
  uint32_t in{0};
  ic_api.from_wire(CandidTypeNat32(&in));
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat32(in));
}

void roundtrip_nat32_1_000_000_001() {
  IC_API ic_api(false);
  uint32_t in{0};
  ic_api.from_wire(CandidTypeNat32(&in));
  if (in != 1000000001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat32(in));
}

void roundtrip_nat64_101() {
  IC_API ic_api(false);

  uint64_t in{0};
  ic_api.from_wire(CandidTypeNat64(&in));
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat64(in));
}

void roundtrip_nat64_1_000_000_000_000_000_001() {
  IC_API ic_api(false);

  uint64_t in{0};
  ic_api.from_wire(CandidTypeNat64(&in));
  if (in != 1000000000000000001)
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeNat64(in));
}

void roundtrip_int8_101() {
  IC_API ic_api(false);
  int8_t in{0};
  ic_api.from_wire(CandidTypeInt8(&in));
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt8(in));
}

void roundtrip_int8_101_neg() {
  IC_API ic_api(false);
  int8_t in{0};
  ic_api.from_wire(CandidTypeInt8(&in));
  if (in != -101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt8(in));
}

void roundtrip_int16_101() {
  IC_API ic_api(false);
  int16_t in{0};
  ic_api.from_wire(CandidTypeInt16(&in));
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt16(in));
}

void roundtrip_int16_101_neg() {
  IC_API ic_api(false);
  int16_t in{0};
  ic_api.from_wire(CandidTypeInt16(&in));
  if (in != -101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt16(in));
}

void roundtrip_int16_1001() {
  IC_API ic_api(false);
  int16_t in{0};
  ic_api.from_wire(CandidTypeInt16(&in));
  if (in != 1001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt16(in));
}

void roundtrip_int16_1001_neg() {
  IC_API ic_api(false);
  int16_t in{0};
  ic_api.from_wire(CandidTypeInt16(&in));
  if (in != -1001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt16(in));
}

void roundtrip_int32_101() {
  IC_API ic_api(false);
  int32_t in{0};
  ic_api.from_wire(CandidTypeInt32(&in));
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt32(in));
}

void roundtrip_int32_101_neg() {
  IC_API ic_api(false);
  int32_t in{0};
  ic_api.from_wire(CandidTypeInt32(&in));
  if (in != -101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt32(in));
}

void roundtrip_int32_1_000_000_001() {
  IC_API ic_api(false);
  int32_t in{0};
  ic_api.from_wire(CandidTypeInt32(&in));
  if (in != 1000000001) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt32(in));
}

void roundtrip_int32_1_000_000_001_neg() {
  IC_API ic_api(false);
  int32_t in{0};
  ic_api.from_wire(CandidTypeInt32(&in));
  if (in != -1000000001)
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt32(in));
}

void roundtrip_int64_101() {
  IC_API ic_api(false);
  int64_t in{0};
  ic_api.from_wire(CandidTypeInt64(&in));
  if (in != 101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt64(in));
}

void roundtrip_int64_101_neg() {
  IC_API ic_api(false);
  int64_t in{0};
  ic_api.from_wire(CandidTypeInt64(&in));
  if (in != -101) IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt64(in));
}

void roundtrip_int64_1_000_000_000_000_000_001() {
  IC_API ic_api(false);
  int64_t in{0};
  ic_api.from_wire(CandidTypeInt64(&in));
  if (in != 1000000000000000001)
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt64(in));
}

void roundtrip_int64_1_000_000_000_000_000_001_neg() {
  IC_API ic_api(false);
  int64_t in{0};
  ic_api.from_wire(CandidTypeInt64(&in));
  if (in != -1000000000000000001)
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeInt64(in));
}

void roundtrip_text() {
  IC_API ic_api(false);
  std::string in{""};
  ic_api.from_wire(CandidTypeText(&in));
  if (in != "Hello C++ Canister")
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeText(in));
}

void roundtrip_text_to_json_to_text() {
  IC_API ic_api(false);

  // Get the Candid Text from the wire
  std::string in{""};
  ic_api.from_wire(CandidTypeText(&in));

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

  ic_api.to_wire(CandidTypeText(s));
}

void roundtrip_text_to_json_to_text_long_message() {
  IC_API ic_api(false);

  // Get the Candid Text from the wire
  std::string in{""};
  ic_api.from_wire(CandidTypeText(&in));

  // Convert it into a nlohmann/json object
  // See: https://json.nlohmann.me/api/basic_json/parse/
  nlohmann::json j = nlohmann::json::parse(in);

  // just return the same thing, as a string
  std::string s = j.dump();

  IC_API::debug_print(
      "roundtrip_text_to_json_to_text_long_message: replying with text:");
  IC_API::debug_print(s);

  ic_api.to_wire(CandidTypeText(s));
}

void roundtrip_reserved() {
  IC_API ic_api(false);
  ic_api.from_wire(CandidTypeReserved());
  ic_api.to_wire(CandidTypeReserved());
}

void roundtrip_float32() {
  IC_API ic_api(false);
  float in{0.0};
  ic_api.from_wire(CandidTypeFloat32(&in));
  if (!is_approximately_equal(in, (float)1001.1001))
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeFloat32(in));
}

void roundtrip_float32_neg() {
  IC_API ic_api(false);
  float in{0.0};
  ic_api.from_wire(CandidTypeFloat32(&in));
  if (!is_approximately_equal(in, (float)-1001.1001))
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeFloat32(in));
}

void roundtrip_float64() {
  IC_API ic_api(false);
  double in{0.0};
  ic_api.from_wire(CandidTypeFloat64(&in));
  if (!is_approximately_equal(in, (double)1001.1001))
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeFloat64(in));
}

void roundtrip_float64_neg() {
  IC_API ic_api(false);
  double in{0.0};
  ic_api.from_wire(CandidTypeFloat64(&in));
  if (!is_approximately_equal(in, (double)-1001.1001))
    IC_API::trap("ASSERT ERROR - " + std::string(__func__));
  ic_api.to_wire(CandidTypeFloat64(in));
}

void roundtrip_record() {
  IC_API ic_api(false);

  // ---------------------------------------------------------------------------
  // Get the data from the wire

  std::string name{""};
  double secret_x{0.0};
  __int128_t secret_i{0};

  CandidTypeRecord r_in;
  r_in.append("name", CandidTypeText(&name));
  r_in.append("secret float64", CandidTypeFloat64(&secret_x));
  r_in.append("secret int", CandidTypeInt(&secret_i));
  //
  std::vector<CandidType> args_in;
  args_in.push_back(r_in);
  //
  ic_api.from_wire(args_in);

  // std::string name = "C++ Developer";
  // double secret_x = 0.01;
  // int secret_i = 11;

  if (name != "C++ Developer")
    IC_API::trap("ASSERT ERROR string - " + std::string(__func__));
  if (!is_approximately_equal(secret_x, (double)0.01))
    IC_API::trap("ASSERT ERROR double - " + std::string(__func__));
  if (secret_i != 11)
    IC_API::trap("ASSERT ERROR int - " + std::string(__func__));
  // ---------------------------------------------------------------------------

  std::string greeting = "Hello " + name + "!";
  std::string message = "Your secret numbers are:";

  CandidTypeRecord record_out;
  record_out.append("greeting", CandidTypeText(greeting));
  record_out.append("message", CandidTypeText(message));
  record_out.append("secret float64", CandidTypeFloat64(secret_x));
  record_out.append("secret int", CandidTypeInt(secret_i));

  ic_api.to_wire(record_out);
}

void test_vendors() {
  IC_API ic_api(false);
  int result = 0;
  result = std::max(result, unit_test_vendors());
  ic_api.to_wire(CandidTypeInt(result));
}

void test_candid() {
  IC_API ic_api(false);
  int result = 0;
  result = std::max(result, unit_test_candid());
  result = std::max(result, compliance_test_prim());
  result = std::max(result, compliance_test_construct());
  ic_api.to_wire(CandidTypeInt(result));
}

void test_ic_api() {
  IC_API ic_api(false);
  int result = unit_test_ic_api();
  ic_api.to_wire(CandidTypeInt(result));
}

void canister_sends_record() {
  IC_API ic_api(false);

  std::string greeting = "Hello!";
  std::string message = "icpp was first release in this month & year:";
  std::string month = "February";
  int year = 2023;

  CandidTypeRecord record_out;
  record_out.append("greeting", CandidTypeText(greeting));
  record_out.append("message", CandidTypeText(message));
  record_out.append("month", CandidTypeText(month));
  record_out.append("year", CandidTypeInt(year));

  ic_api.to_wire(record_out);
}

void canister_sends_int_as_int() {
  IC_API ic_api;

  int n = 101;

  std::string msg;
  msg.append("canister_sends_int_as_int: Replying with int = ");
  msg.append(std::to_string(n));
  IC_API::debug_print(msg);

  // C++ type int = Candid type int
  ic_api.to_wire(CandidTypeInt(n));
}

void canister_sends_char_as_text() {
  IC_API ic_api(false);

  const char *c = "Hello!!!";

  // C++ type const char => Candid type text
  ic_api.to_wire(CandidTypeText(c));
}

void canister_sends_json_as_text() {
  // Example from: https://github.com/nlohmann/json
  IC_API ic_api(false);

  // As STL container
  nlohmann::json j;
  j["happy"] = true;
  j["pi"] = 3.141;

  // Conversion to string
  std::string s = j.dump();

  IC_API::debug_print("canister_sends_json_as_text: replying with text:");
  IC_API::debug_print(s);

  ic_api.to_wire(CandidTypeText(s));
}
