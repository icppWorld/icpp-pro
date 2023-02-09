// Main entry point for a native debug executable.
// Build it with: `icpp build-native` from the parent folder where 'icpp.toml' resides

#include "main.h"

#include <iostream>

#include "../src/my_canister.h"

// The Mock IC
#include "global.h"
#include "mock_ic.h"

int main() {
  MockIC mockIC(true);

  //----------------------------------------------------------------------------------
  // Run all unit tests for vendor libraries

  // '()' -> '(0)'
  mockIC.run_test("test_vendors", test_vendors, "4449444c0000",
                  "4449444c00017c00");

  //----------------------------------------------------------------------------------
  // Run all unit tests

  // '()' -> '(0)'
  mockIC.run_test("test_candid", test_candid, "4449444c0000",
                  "4449444c00017c00");

  // '()' -> '(0)'
  mockIC.run_test("test_ic_api", test_ic_api, "4449444c0000",
                  "4449444c00017c00");

  //----------------------------------------------------------------------------------
  // '(true)' -> '(true)'
  mockIC.run_test("roundtrip_bool", roundtrip_bool, "4449444c00017e01",
                  "4449444c00017e01");
  // '(false)' -> '(false)'
  mockIC.run_test("roundtrip_bool", roundtrip_bool, "4449444c00017e00",
                  "4449444c00017e00");

  // from_wire:
  // '(record {"name" = "C++ Developer"; "secret float64" = 0.01 : float64; "secret int" = 11 : int;})'
  // '(record {1_224_700_491 = "C++ Developer"; 1_274_861_098 = 0.01 : float64; 2_143_348_543 = 11 : int;})'
  // to_wire:
  // '(record {"greeting" = "Hello C++ Developer!"; "message" = "Your secret numbers are:"; "secret float64" = 0.01 : float64; "secret int" = 11 : int;})'
  // '(record {1_274_861_098 = 0.01 : float64; 1_583_063_481 = "Hello C++ Developer!"; 2_143_348_543 = 11 : int; 2_584_819_143 = "Your secret numbers are:";})'
  mockIC.run_test(
      "roundtrip_record", roundtrip_record,
      "4449444c016c03cbe4fdc70471aaacf3df0472bfce83fe077c01000d432b2b20446576656c6f7065727b14ae47e17a843f0b",
      "4449444c016c04aaacf3df0472b9c3eef20571bfce83fe077cc7ebc4d0097101007b14ae47e17a843f1448656c6c6f20432b2b20446576656c6f706572210b18596f757220736563726574206e756d62657273206172653a");

  // When you didc encode, these give same result, due to hashing & sorting:
  // '()' -> '(record {207_603_520 = "February"; 1_346_881_981 = 2_023 : int; 1_583_063_481 = "Hello!"; 2_584_819_143 = "icpp was first release in this month & year:"})'
  // '()' -> '(record {"month" = "February"; "year" = 2_023 : int; "greeting" = "Hello!"; "message" = "icpp was first release in this month & year:"})'
  // '()' -> '(record {"greeting" = "Hello!"; "message" = "icpp was first release in this month & year:"; "month" = "February"; "year" = 2_023 : int;})'
  mockIC.run_test(
      "canister_sends_record", canister_sends_record, "4449444c0000",
      "4449444c016c04c08eff6271bd939f82057cb9c3eef20571c7ebc4d009710100084665627275617279e70f0648656c6c6f212c69637070207761732066697273742072656c6561736520696e2074686973206d6f6e7468202620796561723a");

  // // '()' -> '()'
  // mockIC.run_test("canister_sends_empty", canister_sends_empty, "4449444c0000",
  //                 "4449444c0000");

  // // '()' -> '(101)'
  // mockIC.run_test("canister_sends_int_as_int", canister_sends_int_as_int,
  //                 "4449444c0000", "4449444c00017ce500");

  // // '()' -> '(1001.1001)'
  // mockIC.run_test("canister_sends_double_as_float64", canister_sends_double_as_float64,
  //                 "4449444c0000", "4449444c000172a3923a01cd488f40");

  // '()' -> '("Hello!!!")'
  mockIC.run_test("canister_sends_char_as_text", canister_sends_char_as_text,
                  "4449444c0000", "4449444c0001710848656c6c6f212121");

  // '()' ->'("Hello!!!")'
  mockIC.run_test("canister_sends_string_as_text",
                  canister_sends_string_as_text, "4449444c0000",
                  "4449444c0001710848656c6c6f212121");

  // '()' -> '("{\"happy\":true,\"pi\":3.141}")'
  mockIC.run_test(
      "canister_sends_json_as_text", canister_sends_json_as_text,
      "4449444c0000",
      "4449444c000171197b226861707079223a747275652c227069223a332e3134317d");

  // '("{\"project\": \"?\", \"C++\": 0}")' -> '("{\"C++\":20,\"project\":\"icpp\"}")'
  mockIC.run_test(
      "roundtrip_text_to_json_to_text", roundtrip_text_to_json_to_text,
      "4449444c0001711a7b2270726f6a656374223a20223f222c2022432b2b223a20307d",
      "4449444c0001711b7b22432b2b223a32302c2270726f6a656374223a2269637070227d");

  // '("{\"C++\":20,\"a_long_message\":\"abcdefghijklmnopqrstuvwxyz01abcdefghijklmnopqrstuvwxyz02abcdefghijklmnopqrstuvwxyz03abcdefghijklmnopqrstuvwxyz04abcdefghijklmnopqrstuvwxyz05abcdefghijklmnopqrstuvwxyz06abcdefghijklmnopqrstuvwxyz07abcdefghijklmnopqrstuvwxyz08abcdefghijklmnopqrstuvwxyz09abcdefghijklmnopqrstuvwxyz10abcdefghijklmnopqrstuvwxyz11\",\"project\":\"icpp\"}")'
  mockIC
      .run_test("roundtrip_text_to_json_to_text_long_message",
                roundtrip_text_to_json_to_text_long_message, "4449444c000171e3027b22432b2b223a32302c22615f6c6f6e675f6d657373616765223a226162636465666768696a6b6c6d6e6f707172737475767778797a30316162636465666768696a6b6c6d6e6f707172737475767778797a30326162636465666768696a6b6c6d6e6f707172737475767778797a30336162636465666768696a6b6c6d6e6f707172737475767778797a30346162636465666768696a6b6c6d6e6f707172737475767778797a30356162636465666768696a6b6c6d6e6f707172737475767778797a30366162636465666768696a6b6c6d6e6f707172737475767778797a30376162636465666768696a6b6c6d6e6f707172737475767778797a30386162636465666768696a6b6c6d6e6f707172737475767778797a30396162636465666768696a6b6c6d6e6f707172737475767778797a31306162636465666768696a6b6c6d6e6f707172737475767778797a3131222c2270726f6a656374223a2269637070227d", "4449444c000171e3027b22432b2b223a32302c22615f6c6f6e675f6d657373616765223a226162636465666768696a6b6c6d6e6f707172737475767778797a30316162636465666768696a6b6c6d6e6f707172737475767778797a30326162636465666768696a6b6c6d6e6f707172737475767778797a30336162636465666768696a6b6c6d6e6f707172737475767778797a30346162636465666768696a6b6c6d6e6f707172737475767778797a30356162636465666768696a6b6c6d6e6f707172737475767778797a30366162636465666768696a6b6c6d6e6f707172737475767778797a30376162636465666768696a6b6c6d6e6f707172737475767778797a30386162636465666768696a6b6c6d6e6f707172737475767778797a30396162636465666768696a6b6c6d6e6f707172737475767778797a31306162636465666768696a6b6c6d6e6f707172737475767778797a3131222c2270726f6a656374223a2269637070227d");

  // returns 1 if any tests failed
  return mockIC.test_summary();
}
