// Main entry point for a native debug executable.
// Build it with: `icpp build-native` from the parent folder where 'icpp.toml' resides

#include "main.h"

#include <iostream>

#include "../src/my_canister.h"

// The Mock IC
#include "icpp_hooks.h"
#include "mock_ic.h"

int main() {
  bool exit_on_fail = true;
  MockIC mockIC(exit_on_fail);

  // The pretend principals of the caller
  std::string my_principal{
      "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae"};
  std::string anonymous_principal{"2vxsx-fae"};

  bool silent_on_trap = true;

  //----------------------------------------------------------------------------------
  // Run all unit tests for vendor libraries

  // '()' -> '(0)'
  mockIC.run_test("test_vendors", test_vendors, "4449444c0000",
                  "4449444c00017c00", silent_on_trap, my_principal);

  //----------------------------------------------------------------------------------
  // Run all unit & compliance tests for candid

  // '()' -> '(0)'
  mockIC.run_test("test_candid", test_candid, "4449444c0000",
                  "4449444c00017c00", silent_on_trap, my_principal);

  //----------------------------------------------------------------------------------
  // Run all unit tests for ic_api

  // '()' -> '(0)'
  mockIC.run_test("test_ic_api", test_ic_api, "4449444c0000",
                  "4449444c00017c00", silent_on_trap, my_principal);

  //----------------------------------------------------------------------------------
  // Run all roundtrip tests

  // '(true)' -> '(true)'
  mockIC.run_test("roundtrip_bool_true", roundtrip_bool_true,
                  "4449444c00017e01", "4449444c00017e01", silent_on_trap,
                  my_principal);
  // '(false)' -> '(false)'
  mockIC.run_test("roundtrip_bool_false", roundtrip_bool_false,
                  "4449444c00017e00", "4449444c00017e00", silent_on_trap,
                  my_principal);

  // '(101 : nat)' -> '(101 : nat)'
  mockIC.run_test("roundtrip_nat_101", roundtrip_nat_101, "4449444c00017d65",
                  "4449444c00017d65", silent_on_trap, my_principal);
  // '(1001 : nat)' -> '(1001 : nat)'
  mockIC.run_test("roundtrip_nat_1001", roundtrip_nat_1001,
                  "4449444c00017de907", "4449444c00017de907", silent_on_trap,
                  my_principal);

  // '(101 : int)' -> '(101 : int)'
  mockIC.run_test("roundtrip_int_101", roundtrip_int_101, "4449444c00017ce500",
                  "4449444c00017ce500", silent_on_trap, my_principal);
  // '(1001 : int)' -> '(1001 : int)'
  mockIC.run_test("roundtrip_int_1001", roundtrip_int_1001,
                  "4449444c00017ce907", "4449444c00017ce907", silent_on_trap,
                  my_principal);
  // '(-101 : int)' -> '(-101 : int)'
  mockIC.run_test("roundtrip_int_101_neg", roundtrip_int_101_neg,
                  "4449444c00017c9b7f", "4449444c00017c9b7f", silent_on_trap,
                  my_principal);
  // '(-1001 : int)' -> '(-1001 : int)'
  mockIC.run_test("roundtrip_int_1001_neg", roundtrip_int_1001_neg,
                  "4449444c00017c9778", "4449444c00017c9778", silent_on_trap,
                  my_principal);

  // '(101 : nat8)' -> '(101 : nat8)'
  mockIC.run_test("roundtrip_nat8_101", roundtrip_nat8_101, "4449444c00017b65",
                  "4449444c00017b65", silent_on_trap, my_principal);

  // '(101 : nat16)' -> '(101 : nat16)'
  mockIC.run_test("roundtrip_nat16_101", roundtrip_nat16_101,
                  "4449444c00017a6500", "4449444c00017a6500", silent_on_trap,
                  my_principal);
  // '(1001 : nat16)' -> '(1001 : nat16)'
  mockIC.run_test("roundtrip_nat16_1001", roundtrip_nat16_1001,
                  "4449444c00017ae903", "4449444c00017ae903", silent_on_trap,
                  my_principal);

  // '(101 : nat32)' -> '(101 : nat32)'
  mockIC.run_test("roundtrip_nat32_101", roundtrip_nat32_101,
                  "4449444c00017965000000", "4449444c00017965000000",
                  silent_on_trap, my_principal);
  // '(1_000_000_001 : nat32)' -> '(1_000_000_001 : nat32)'
  mockIC.run_test("roundtrip_nat32_1_000_000_001",
                  roundtrip_nat32_1_000_000_001, "4449444c00017901ca9a3b",
                  "4449444c00017901ca9a3b", silent_on_trap, my_principal);

  // '(101 : nat64)' -> '(101 : nat64)'
  mockIC.run_test("roundtrip_nat64_101", roundtrip_nat64_101,
                  "4449444c0001786500000000000000",
                  "4449444c0001786500000000000000", silent_on_trap,
                  my_principal);
  // '(1_000_000_000_000_000_001 : nat64)' -> '(1_000_000_000_000_000_001 : nat64)'
  mockIC.run_test("roundtrip_nat64_1_000_000_000_000_000_001",
                  roundtrip_nat64_1_000_000_000_000_000_001,
                  "4449444c000178010064a7b3b6e00d",
                  "4449444c000178010064a7b3b6e00d", silent_on_trap,
                  my_principal);

  // '(101 : int8)' -> '(101 : int8)'
  mockIC.run_test("roundtrip_int8_101", roundtrip_int8_101, "4449444c00017765",
                  "4449444c00017765", silent_on_trap, my_principal);
  // '(-101 : int8)' -> '(-101 : int8)'
  mockIC.run_test("roundtrip_int8_101_neg", roundtrip_int8_101_neg,
                  "4449444c0001779b", "4449444c0001779b", silent_on_trap,
                  my_principal);

  // '(101 : int16)' -> '(101 : int16)'
  mockIC.run_test("roundtrip_int16_101", roundtrip_int16_101,
                  "4449444c0001766500", "4449444c0001766500", silent_on_trap,
                  my_principal);
  // '(1001 : int16)' -> '(1001 : int16)'
  mockIC.run_test("roundtrip_int16_1001", roundtrip_int16_1001,
                  "4449444c000176e903", "4449444c000176e903", silent_on_trap,
                  my_principal);
  // '(-101 : int16)' -> '(-101 : int16)'
  mockIC.run_test("roundtrip_int16_101_neg", roundtrip_int16_101_neg,
                  "4449444c0001769bff", "4449444c0001769bff", silent_on_trap,
                  my_principal);
  // '(-1001 : int16)' -> '(-1001 : int16)'
  mockIC.run_test("roundtrip_int16_1001_neg", roundtrip_int16_1001_neg,
                  "4449444c00017617fc", "4449444c00017617fc", silent_on_trap,
                  my_principal);

  // '(101 : int32)' -> '(101 : int32)'
  mockIC.run_test("roundtrip_int32_101", roundtrip_int32_101,
                  "4449444c00017565000000", "4449444c00017565000000",
                  silent_on_trap, my_principal);
  // '(1_000_000_001 : int32)' -> '(1_000_000_001 : int32)'
  mockIC.run_test("roundtrip_int32_1_000_000_001",
                  roundtrip_int32_1_000_000_001, "4449444c00017501ca9a3b",
                  "4449444c00017501ca9a3b", silent_on_trap, my_principal);
  // '(-101 : int32)' -> '(-101 : int32)'
  mockIC.run_test("roundtrip_int32_101_neg", roundtrip_int32_101_neg,
                  "4449444c0001759bffffff", "4449444c0001759bffffff",
                  silent_on_trap, my_principal);
  // '(-1_000_000_001 : int32)' -> '(-1_000_000_001 : int32)'
  mockIC.run_test("roundtrip_int32_1_000_000_001_neg",
                  roundtrip_int32_1_000_000_001_neg, "4449444c000175ff3565c4",
                  "4449444c000175ff3565c4", silent_on_trap, my_principal);

  // '(101 : int64)' -> '(101 : int64)'
  mockIC.run_test("roundtrip_int64_101", roundtrip_int64_101,
                  "4449444c0001746500000000000000",
                  "4449444c0001746500000000000000", silent_on_trap,
                  my_principal);
  // '(1_000_000_000_000_000_001 : int64)' -> '(1_000_000_000_000_000_001 : int64)'
  mockIC.run_test("roundtrip_int64_1_000_000_000_000_000_001",
                  roundtrip_int64_1_000_000_000_000_000_001,
                  "4449444c000174010064a7b3b6e00d",
                  "4449444c000174010064a7b3b6e00d", silent_on_trap,
                  my_principal);
  // '(-101 : int64)' -> '(-101 : int64)'
  mockIC.run_test("roundtrip_int64_101_neg", roundtrip_int64_101_neg,
                  "4449444c0001749bffffffffffffff",
                  "4449444c0001749bffffffffffffff", silent_on_trap,
                  my_principal);
  // '(-1_000_000_000_000_000_001 : int64)' -> '(-1_000_000_000_000_000_001 : int64)'
  mockIC.run_test("roundtrip_int64_1_000_000_000_000_000_001_neg",
                  roundtrip_int64_1_000_000_000_000_000_001_neg,
                  "4449444c000174ffff9b584c491ff2",
                  "4449444c000174ffff9b584c491ff2", silent_on_trap,
                  my_principal);

  // '("Hello C++ Canister")' ->'("Hello C++ Canister")'
  mockIC.run_test("roundtrip_text", roundtrip_text,
                  "4449444c0001711248656c6c6f20432b2b2043616e6973746572",
                  "4449444c0001711248656c6c6f20432b2b2043616e6973746572",
                  silent_on_trap, my_principal);
  // '("{\"project\": \"?\", \"C++\": 0}")' -> '("{\"C++\":20,\"project\":\"icpp\"}")'
  mockIC.run_test(
      "roundtrip_text_to_json_to_text", roundtrip_text_to_json_to_text,
      "4449444c0001711a7b2270726f6a656374223a20223f222c2022432b2b223a20307d",
      "4449444c0001711b7b22432b2b223a32302c2270726f6a656374223a2269637070227d",
      silent_on_trap, my_principal);

  // '("{\"C++\":20,\"a_long_message\":\"abcdefghijklmnopqrstuvwxyz01abcdefghijklmnopqrstuvwxyz02abcdefghijklmnopqrstuvwxyz03abcdefghijklmnopqrstuvwxyz04abcdefghijklmnopqrstuvwxyz05abcdefghijklmnopqrstuvwxyz06abcdefghijklmnopqrstuvwxyz07abcdefghijklmnopqrstuvwxyz08abcdefghijklmnopqrstuvwxyz09abcdefghijklmnopqrstuvwxyz10abcdefghijklmnopqrstuvwxyz11\",\"project\":\"icpp\"}")'
  mockIC.run_test(
      "roundtrip_text_to_json_to_text_long_message",
      roundtrip_text_to_json_to_text_long_message,
      "4449444c000171e3027b22432b2b223a32302c22615f6c6f6e675f6d657373616765223a226162636465666768696a6b6c6d6e6f707172737475767778797a30316162636465666768696a6b6c6d6e6f707172737475767778797a30326162636465666768696a6b6c6d6e6f707172737475767778797a30336162636465666768696a6b6c6d6e6f707172737475767778797a30346162636465666768696a6b6c6d6e6f707172737475767778797a30356162636465666768696a6b6c6d6e6f707172737475767778797a30366162636465666768696a6b6c6d6e6f707172737475767778797a30376162636465666768696a6b6c6d6e6f707172737475767778797a30386162636465666768696a6b6c6d6e6f707172737475767778797a30396162636465666768696a6b6c6d6e6f707172737475767778797a31306162636465666768696a6b6c6d6e6f707172737475767778797a3131222c2270726f6a656374223a2269637070227d",
      "4449444c000171e3027b22432b2b223a32302c22615f6c6f6e675f6d657373616765223a226162636465666768696a6b6c6d6e6f707172737475767778797a30316162636465666768696a6b6c6d6e6f707172737475767778797a30326162636465666768696a6b6c6d6e6f707172737475767778797a30336162636465666768696a6b6c6d6e6f707172737475767778797a30346162636465666768696a6b6c6d6e6f707172737475767778797a30356162636465666768696a6b6c6d6e6f707172737475767778797a30366162636465666768696a6b6c6d6e6f707172737475767778797a30376162636465666768696a6b6c6d6e6f707172737475767778797a30386162636465666768696a6b6c6d6e6f707172737475767778797a30396162636465666768696a6b6c6d6e6f707172737475767778797a31306162636465666768696a6b6c6d6e6f707172737475767778797a3131222c2270726f6a656374223a2269637070227d",
      silent_on_trap, my_principal);

  // '(null : reserved)' -> (null : reserved)
  mockIC.run_test("roundtrip_reserved", roundtrip_reserved, "4449444c000170",
                  "4449444c000170", silent_on_trap, my_principal);

  // '(1001.1001 : float32)' -> '(1001.1001 : float32)'
  mockIC.run_test("roundtrip_float32", roundtrip_float32,
                  "4449444c00017368467a44", "4449444c00017368467a44",
                  silent_on_trap, my_principal);
  // '(-1001.1001 : float32)' -> '(-1001.1001 : float32)'
  mockIC.run_test("roundtrip_float32_neg", roundtrip_float32_neg,
                  "4449444c00017368467ac4", "4449444c00017368467ac4",
                  silent_on_trap, my_principal);

  // '(1001.1001 : float64)' -> '(1001.1001 : float64)'
  mockIC.run_test(
      "roundtrip_float64", roundtrip_float64, "4449444c000172a3923a01cd488f40",
      "4449444c000172a3923a01cd488f40", silent_on_trap, my_principal);
  // '(-1001.1001 : float64)' -> '(-1001.1001 : float64)'
  mockIC.run_test("roundtrip_float64_neg", roundtrip_float64_neg,
                  "4449444c000172a3923a01cd488fc0",
                  "4449444c000172a3923a01cd488fc0", silent_on_trap,
                  my_principal);

  // '(principal "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae")' -> '(principal "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae")'
  mockIC.run_test(
      "roundtrip_principal", roundtrip_principal,
      "4449444c000168011d779590d2cd339802981dfd935d9a3dbb085cafe6ad19b87229a016d602",
      "4449444c000168011d779590d2cd339802981dfd935d9a3dbb085cafe6ad19b87229a016d602",
      silent_on_trap, my_principal);

  // '()' -> '(false : bool)'
  mockIC.run_test("caller_is_anonymous", caller_is_anonymous, "4449444c0000",
                  "4449444c00017e00", silent_on_trap, my_principal);

  // '()' -> '(true : bool)'
  mockIC.run_test("caller_is_anonymous", caller_is_anonymous, "4449444c0000",
                  "4449444c00017e01", silent_on_trap, anonymous_principal);

  // '(vec { 101 : nat16; 102 : nat16; 103 : nat16 })' -> '(vec { 101 : nat16; 102 : nat16; 103 : nat16 })'
  mockIC.run_test("roundtrip_vec_nat16", roundtrip_vec_nat16,
                  "4449444c016d7a010003650066006700",
                  "4449444c016d7a010003650066006700", silent_on_trap,
                  my_principal);

  // '(vec { true : bool; false : bool }, vec { 101 : nat; 102 : nat; 103 : nat }, vec { 101 : nat8; 102 : nat8; 103 : nat8 }, vec { 101 : nat16; 102 : nat16; 103 : nat16 }, vec { 101 : nat32; 102 : nat32; 103 : nat32 }, vec { 101 : nat64; 102 : nat64; 103 : nat64 }, vec { -101 : int; -102 : int; -103 : int }, vec { -101 : int8; -102 : int8; -103 : int8 }, vec { -101 : int16; -102 : int16; -103 : int16 }, vec { -101 : int32; -102 : int32; -103 : int32 }, vec { -101 : int64; -102 : int64; -103 : int64 }, vec { -1.01 : float32; -1.02 : float32; -1.03 : float32 }, vec { -1.01 : float64; -1.02 : float64; -1.03 : float64 }, vec { "Hello 101" : text; "Hello 102" : text; "Hello 103" : text }, vec { principal "2ibo7-dia"; principal "w3gef-eqbai"; principal "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae"})'
  // -> same
  mockIC.run_test(
      "roundtrip_vec_all", roundtrip_vec_all,
      "4449444c0f6d7e6d7d6d7b6d7a6d796d786d7c6d776d766d756d746d736d726d716d680f000102030405060708090a0b0c0d0e0201000365666703656667036500660067000365000000660000006700000003650000000000000066000000000000006700000000000000039b7f9a7f997f039b9a99039bff9aff99ff039bffffff9affffff99ffffff039bffffffffffffff9affffffffffffff99ffffffffffffff03ae4781bf5c8f82bf0ad783bf03295c8fc2f528f0bf52b81e85eb51f0bf7b14ae47e17af0bf030948656c6c6f203130310948656c6c6f203130320948656c6c6f203130330301010001020102011d779590d2cd339802981dfd935d9a3dbb085cafe6ad19b87229a016d602",
      "4449444c0f6d7e6d7d6d7b6d7a6d796d786d7c6d776d766d756d746d736d726d716d680f000102030405060708090a0b0c0d0e0201000365666703656667036500660067000365000000660000006700000003650000000000000066000000000000006700000000000000039b7f9a7f997f039b9a99039bff9aff99ff039bffffff9affffff99ffffff039bffffffffffffff9affffffffffffff99ffffffffffffff03ae4781bf5c8f82bf0ad783bf03295c8fc2f528f0bf52b81e85eb51f0bf7b14ae47e17af0bf030948656c6c6f203130310948656c6c6f203130320948656c6c6f203130330301010001020102011d779590d2cd339802981dfd935d9a3dbb085cafe6ad19b87229a016d602",
      silent_on_trap, my_principal);

  // '(opt (101 : nat))' -> same
  mockIC.run_test("roundtrip_opt_nat", roundtrip_opt_nat,
                  "4449444c016e7d01000165", "4449444c016e7d01000165",
                  silent_on_trap, my_principal);

  // '(null)' -> The raw bytes returned represent an opt : nat without a value.
  //             There is no IDL representation for this.
  //             didc decodes it to '(null)'
  mockIC.run_test("roundtrip_opt_nat_no_value", roundtrip_opt_nat_no_value,
                  "4449444c00017f", "4449444c016e7d010000", silent_on_trap,
                  my_principal);
  mockIC.run_test("roundtrip_opt_nat_no_value", roundtrip_opt_nat_no_value,
                  "4449444c016e7d010000", "4449444c016e7d010000",
                  silent_on_trap, my_principal);

  // '(opt (101 : nat16))' -> same
  mockIC.run_test("roundtrip_opt_nat16", roundtrip_opt_nat16,
                  "4449444c016e7a0100016500", "4449444c016e7a0100016500",
                  silent_on_trap, my_principal);

  // '(null)' -> The raw bytes returned represent an opt : nat16 without a value.
  //             There is no IDL representation for this.
  //             didc decodes it to '(null)'
  mockIC.run_test("roundtrip_opt_nat16_no_value", roundtrip_opt_nat16_no_value,
                  "4449444c00017f", "4449444c016e7a010000", silent_on_trap,
                  my_principal);
  mockIC.run_test("roundtrip_opt_nat16_no_value", roundtrip_opt_nat16_no_value,
                  "4449444c016e7a010000", "4449444c016e7a010000",
                  silent_on_trap, my_principal);

  // '(opt (true : bool), opt (false : bool), null, opt (101 : nat), null, opt (102 : nat8), null, opt (103 : nat16), null, opt (104 : nat32), null, opt (105 : nat64), null, opt (-101 : int), null, opt (-102 : int8), null, opt (-103 : int16), null, opt (-104 : int32), null, opt (-105 : int64), null, opt (-1.01 : float32), null, opt (-1.02 : float64), null, opt ("Hello 101" : text), null, opt (principal "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae"), null)'
  //-> raw bytes are not using null, but an opt:natXX without a value, as explained in previous
  mockIC.run_test(
      "roundtrip_opt_all", roundtrip_opt_all,
      "4449444c0f6e7e6e7d6e7b6e7a6e796e786e7c6e776e766e756e746e736e726e716e681f00007f017f027f037f047f057f067f077f087f097f0a7f0b7f0c7f0d7f0e7f01010100016501660167000168000000016900000000000000019b7f019a0199ff0198ffffff0197ffffffffffffff01ae4781bf0152b81e85eb51f0bf010948656c6c6f2031303101011d779590d2cd339802981dfd935d9a3dbb085cafe6ad19b87229a016d602",
      "4449444c0f6e7e6e7d6e7b6e7a6e796e786e7c6e776e766e756e746e736e726e716e681f0000000101020203030404050506060707080809090a0a0b0b0c0c0d0d0e0e01010100000165000166000167000001680000000001690000000000000000019b7f00019a000199ff000198ffffff000197ffffffffffffff0001ae4781bf000152b81e85eb51f0bf00010948656c6c6f203130310001011d779590d2cd339802981dfd935d9a3dbb085cafe6ad19b87229a016d60200",
      silent_on_trap, my_principal);
  mockIC.run_test(
      "roundtrip_opt_all", roundtrip_opt_all,
      "4449444c0f6e7e6e7d6e7b6e7a6e796e786e7c6e776e766e756e746e736e726e716e681f0000000101020203030404050506060707080809090a0a0b0b0c0c0d0d0e0e01010100000165000166000167000001680000000001690000000000000000019b7f00019a000199ff000198ffffff000197ffffffffffffff0001ae4781bf000152b81e85eb51f0bf00010948656c6c6f203130310001011d779590d2cd339802981dfd935d9a3dbb085cafe6ad19b87229a016d60200",
      "4449444c0f6e7e6e7d6e7b6e7a6e796e786e7c6e776e766e756e746e736e726e716e681f0000000101020203030404050506060707080809090a0a0b0b0c0c0d0d0e0e01010100000165000166000167000001680000000001690000000000000000019b7f00019a000199ff000198ffffff000197ffffffffffffff0001ae4781bf000152b81e85eb51f0bf00010948656c6c6f203130310001011d779590d2cd339802981dfd935d9a3dbb085cafe6ad19b87229a016d60200",
      silent_on_trap, my_principal);

  // from_wire:
  // '(record {"name" = "C++ Developer"; "secret float64" = 0.01 : float64; "secret int" = 11 : int;})'
  // '(record {1_224_700_491 = "C++ Developer"; 1_274_861_098 = 0.01 : float64; 2_143_348_543 = 11 : int;})'
  // to_wire:
  // '(record {"greeting" = "Hello C++ Developer!"; "message" = "Your secret numbers are:"; "secret float64" = 0.01 : float64; "secret int" = 11 : int;})'
  // '(record {1_274_861_098 = 0.01 : float64; 1_583_063_481 = "Hello C++ Developer!"; 2_143_348_543 = 11 : int; 2_584_819_143 = "Your secret numbers are:";})'
  mockIC.run_test(
      "roundtrip_record", roundtrip_record,
      "4449444c016c03cbe4fdc70471aaacf3df0472bfce83fe077c01000d432b2b20446576656c6f7065727b14ae47e17a843f0b",
      "4449444c016c04aaacf3df0472b9c3eef20571bfce83fe077cc7ebc4d0097101007b14ae47e17a843f1448656c6c6f20432b2b20446576656c6f706572210b18596f757220736563726574206e756d62657273206172653a",
      silent_on_trap, my_principal);

  // Candid's Variant has two options for the Type Table
  // (1) only include the type table of the variant's value
  // (2) include the type table of all the variant's cases

  // '(variant { Ok })' -> same
  mockIC.run_test("roundtrip_variant_ok (1)", roundtrip_variant_ok,
                  "4449444c016b01bc8a017f010000", // (1)
                  "4449444c016b01bc8a017f010000", silent_on_trap,
                  my_principal);                  // (2)
  mockIC.run_test("roundtrip_variant_ok (2)", roundtrip_variant_ok,
                  "4449444c016b02bc8a017fc5fed20171010000",
                  "4449444c016b01bc8a017f010000", silent_on_trap, my_principal);

  // '(variant { Err = "Error" : text})' -> same
  mockIC.run_test("roundtrip_variant_err (1)", roundtrip_variant_err,
                  "4449444c016b01c5fed20171010000054572726f72", // (1)
                  "4449444c016b01c5fed20171010000054572726f72", silent_on_trap,
                  my_principal);                                // (2)
  mockIC.run_test("roundtrip_variant_err (2)", roundtrip_variant_err,
                  "4449444c016b02bc8a0171c5fed20171010001054572726f72",
                  "4449444c016b01c5fed20171010000054572726f72", silent_on_trap,
                  my_principal);

  // See my_canister.did for the custom type definitions
  // from_wire:
  //   Result          Result                        Result_1                           Result_1                      Result_2                           Result_2
  // '(variant { Ok }, variant { Err = 404 : nat16}, variant { Ok = "All good" : text}, variant { Err = 500 : nat16}, variant { Ok = "All good" : text}, variant { Err = "Error" : text})'
  // to_wire: same
  /* didc decode shows the hashed ids:
  (
    variant { 17_724 },
    variant { 3_456_837 = 404 : nat16 },
    variant { 17_724 = "All good" },
    variant { 3_456_837 = 500 : nat16 },
    variant { 17_724 = "All good" },
    variant { 3_456_837 = "Error" },
  )
  */
  mockIC.run_test(
      "roundtrip_variant", roundtrip_variant,
      "4449444c046b01bc8a017f6b01c5fed2017a6b01bc8a01716b01c5fed2017106000102010203000094010008416c6c20676f6f6400f4010008416c6c20676f6f6400054572726f72", // (1)
      "4449444c036b02bc8a017fc5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed2017106000001010202000194010008416c6c20676f6f6401f4010008416c6c20676f6f6401054572726f72",
      silent_on_trap, my_principal); // (2)
  mockIC.run_test(
      "roundtrip_variant", roundtrip_variant,
      "4449444c036b02bc8a017fc5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed2017106000001010202000194010008416c6c20676f6f6401f4010008416c6c20676f6f6401054572726f72", // (1)
      "4449444c036b02bc8a017fc5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed2017106000001010202000194010008416c6c20676f6f6401f4010008416c6c20676f6f6401054572726f72",
      silent_on_trap, my_principal); // (2)

  // ----------------------------------------------------------------------------------
  // Trap testing
  //

  // Verify that a Deserialization traps if the number of arguments is wrong
  // '(true, true)' goes in, but it only expects one '(true)'
  mockIC.run_trap_test("trap_wrong_number_of_args", trap_wrong_number_of_args,
                       "4449444c00027e7e0101", silent_on_trap);

  // Verify that a canister traps if from_wire is called more than once
  // '(true)' -> '(true)'  (But it never returns, it traps)
  mockIC.run_trap_test("trap_multiple_calls_from_wire",
                       trap_multiple_calls_from_wire, "4449444c00017e01",
                       silent_on_trap);

  // Verify that a canister traps if to_wire is called more than once
  // '(true)' -> '(true)'  (But it never returns, it traps)
  mockIC.run_trap_test("trap_multiple_calls_to_wire",
                       trap_multiple_calls_to_wire, "4449444c00017e01",
                       silent_on_trap);

  // Verify that a Record traps on hash collission
  try {
    CandidTypeRecord r;
    r.append("syndactyle", CandidTypeText("some text"));
    r.append("rectum", CandidTypeText("another text"));
    ICPP_HOOKS::trap(std::string(__func__) + ": did not trap 1a");
  } catch (const std::exception &e) {
  }

  // Verify that a record traps if one of the records's id (hash) on wire does not match expected
  // Expecting:
  // '(record {"name" = "C++ Developer"; "secret float64" = 0.01 : float64; "secret int" = 11 : int;})'
  // '(record {1_224_700_491 = "C++ Developer"; 1_274_861_098 = 0.01 : float64; 2_143_348_543 = 11 : int;})'
  // Sending
  // '(record {"name" = "C++ Developer"; "public float64" = 0.01 : float64; "secret int" = 11 : int;})'
  mockIC.run_trap_test(
      "roundtrip_record (trap)", roundtrip_record,
      "4449444c016c03e3d2a2c10172cbe4fdc70471bfce83fe077c01007b14ae47e17a843f0d432b2b20446576656c6f7065720b",
      silent_on_trap);

  // Verify that a variant traps if the variant's id (hash) on wire does not match expected
  // '(variant { Blah = "BlahBlah" : text})' goes in, but it expects a variant option "Err"
  mockIC.run_trap_test("roundtrip_variant_err (trap 1)", roundtrip_variant_err,
                       "4449444c016b01d1bac9df027101000008426c6168426c6168",
                       silent_on_trap);

  // Verify that a variant traps if the variant's id (hash) on wire is ok, but type does not match
  // '(variant { Err = 1 : nat16})' goes in, but it expects a variant option "Err" : text
  mockIC.run_trap_test("roundtrip_variant_err  (trap 1)", roundtrip_variant_err,
                       "4449444c016b01c5fed2017a0100000100", silent_on_trap);

  //
  // Verify wrong text representations of of Principal
  // https://internetcomputer.org/docs/current/references/id-encoding-spec#decode
  // {
  //   std::string s = "w3gef-eqbaj";
  //   CandidTypePrincipal p = CandidTypePrincipal(s);
  // }
  try {
    std::string s =
        "aaaaa-aaaaa-bbbbb-bbbbb-22222-22222-33333-33333-44444-44444-5555";
    CandidTypePrincipal p = CandidTypePrincipal(s);
    ICPP_HOOKS::trap(std::string(__func__) + ": did not trap 2a");
  } catch (const std::exception &e) {
  }
  try {
    std::string s =
        "aaaaa-aaaaa-bbbbb-bbbbb-22222-22222-33333-33333-44444-44444-555";
    CandidTypePrincipal p = CandidTypePrincipal(s);
    ICPP_HOOKS::trap(std::string(__func__) + ": did not trap 2b");
  } catch (const std::exception &e) {
  }
  try {
    std::string s = "a2345-678";
    CandidTypePrincipal p = CandidTypePrincipal(s);
    ICPP_HOOKS::trap(std::string(__func__) + ": did not trap 2c");
  } catch (const std::exception &e) {
  }
  try {
    std::string s = "2ibo7-dib";
    CandidTypePrincipal p = CandidTypePrincipal(s);
    ICPP_HOOKS::trap(std::string(__func__) + ": did not trap 2d");
  } catch (const std::exception &e) {
  }
  try {
    std::string s = "2ibo7dia";
    CandidTypePrincipal p = CandidTypePrincipal(s);
    ICPP_HOOKS::trap(std::string(__func__) + ": did not trap 2e");
  } catch (const std::exception &e) {
  }
  try {
    std::string s = "2ibo-7dia";
    CandidTypePrincipal p = CandidTypePrincipal(s);
    ICPP_HOOKS::trap(std::string(__func__) + ": did not trap 2f");
  } catch (const std::exception &e) {
  }
  try {
    std::string s = "2ibo7--dia";
    CandidTypePrincipal p = CandidTypePrincipal(s);
    ICPP_HOOKS::trap(std::string(__func__) + ": did not trap 2g");
  } catch (const std::exception &e) {
  }
  // returns 1 if any tests failed
  return mockIC.test_summary();
}
