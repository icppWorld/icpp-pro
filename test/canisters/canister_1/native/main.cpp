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

  // PATCH START

  // ------------------------------------------------------------------------
  // TODO:  Implement VecVariant
  //

  // ------------------------------------------------------------------------
  // Debugging this existing test
  // ...

  // ----
  // Forward compatibility

  // ------------------------------------------------------------------------

  // ------------------------------------------------------------------------
  //----------------------------------------------------------------------------------
  // Run all canister tests

  // '()' -> '(record {
  //                    canister_id = "5ugrv-zqaaa-aaaag-acfna-cai" : text;
  //                    canister_cycle_balance = 3_000_000_000_000 : nat;
  //                  }
  //         )'
  mockIC.run_test(
      "get_canister_info", get_canister_info, "4449444c0000",
      "4449444c016c02b3c4b1f20471ebe3c4e30a7d01001b35756772762d7a716161612d61616161672d6163666e612d63616980e0bcefa757",
      silent_on_trap, my_principal);

  // TODO
  //   // opt ( WHATEVER ) should never trap, but always decode things to null
  //   //                  if type within opt does not match
  //   // (See forum: https://forum.dfinity.org/t/question-about-record-variant-with-opt-fields-in-light-of-forward-compatibility/24567/2?u=icpp)
  //   // The general rule for opt is that it becomes null if the types cannot be
  //   // made to match otherwise. That happens at the innermost opt where this
  //   // fallback suffices to make the whole thing match.
  //   //
  //   // Canister expects : '(opt ( variant { Ok } ))'
  //   // Canister receives: '(opt ( variant { New } ))'
  //   //                    Must be decoded to null
  //   // -> '()'
  //   mockIC.run_test("roundtrip_opt_variant_ok_no_value fwc",
  //                   roundtrip_opt_variant_ok_no_value,
  //                   "4449444c026e016b01c090ee017f01000100", "4449444c0000",
  //                   silent_on_trap, my_principal);

  //   // Expecting: '(opt ( record {field1 = "hello" : text; field2 = variant {fieldv = 16 : int}; field3 = "bye" : text; } ))'
  //   // Receiving: '(opt ( record {field0 = "WrongId" : text} ))'
  //   //            type does not match. Do NOT trap, but decode to null (has_value=false)
  //   // -> '()'
  //   mockIC.run_test("roundtrip_opt_record_variant_no_value 1",
  //                   roundtrip_opt_record_variant_no_value,
  //                   "4449444c026e016c01b69cba8408710100010757726f6e674964",
  //                   "4449444c0000", silent_on_trap, my_principal);
  //   // Expecting: '(opt ( record {field1 = "hello" : text; field2 = variant {fieldv = 16 : int}; field3 = "bye" : text; } ))'
  //   // Receiving: '(opt ( record {field1 = "hello" : text; field2 = variant {fieldv0 = 16 : int}; field3 = "bye" : text; } ))'
  //   //                                                                       ^wrong!
  //   //            type does not match. Do NOT trap, but decode to null (has_value=false)
  //   // -> '()'
  //   mockIC.run_test(
  //       "roundtrip_opt_record_variant_no_value 2",
  //       roundtrip_opt_record_variant_no_value,
  //       "4449444c036e016c03b79cba840871b89cba840802b99cba8408716b01b4bcb8e10f7c0100010568656c6c6f001003627965",
  //       "4449444c0000", silent_on_trap, my_principal);

  // PATCH END

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
  mockIC.run_test("caller_is_anonymous false", caller_is_anonymous,
                  "4449444c0000", "4449444c00017e00", silent_on_trap,
                  my_principal);

  // '()' -> '(true : bool)'
  mockIC.run_test("caller_is_anonymous true", caller_is_anonymous,
                  "4449444c0000", "4449444c00017e01", silent_on_trap,
                  anonymous_principal);

  // '()' -> '(false : bool)'
  mockIC.run_test("caller_is_controller true", caller_is_controller,
                  "4449444c0000", "4449444c00017e00", silent_on_trap,
                  anonymous_principal);

  // '()' -> '(true : bool)'
  mockIC.run_test("caller_is_controller false", caller_is_controller,
                  "4449444c0000", "4449444c00017e01", silent_on_trap,
                  my_principal);

  // '(vec { 101 : nat16; 102 : nat16; 103 : nat16 })' -> '(vec { 101 : nat16; 102 : nat16; 103 : nat16 })'
  mockIC.run_test("roundtrip_vec_nat16", roundtrip_vec_nat16,
                  "4449444c016d7a010003650066006700",
                  "4449444c016d7a010003650066006700", silent_on_trap,
                  my_principal);

  // '(vec { record {name = "H1N" : text; value = "H1V"}; record {name = "H2N" : text; value = "H2V"}; record {name = "H3N" : text; value = "H3V"}; })'
  // -> same
  // Use compact type table of didc
  mockIC.run_test(
      "roundtrip_vec_record", roundtrip_vec_record,
      "4449444c026d016c02f1fee18d0371cbe4fdc70471010003034831560348314e034832560348324e034833560348334e",
      "4449444c086c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc70471010403034831560348314e034832560348324e034833560348334e",
      silent_on_trap, my_principal);
  // Use larger type table of icpp
  mockIC.run_test(
      "roundtrip_vec_record 2", roundtrip_vec_record,
      "4449444c086c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc70471010403034831560348314e034832560348324e034833560348334e",
      "4449444c086c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc70471010403034831560348314e034832560348324e034833560348334e",
      silent_on_trap, my_principal);

  // '(vec { variant { AdminQuery }; variant { AdminUpdate }; variant { AdminQuery } })'
  // -> same (icpp generates a more verbose type table)
  // Use compact type table of didc
  mockIC.run_test(
      "roundtrip_vec_variant", roundtrip_vec_variant,
      "4449444c026d016b0299d8ddf2087fd8a7f6ae0b7f010003000100",
      "4449444c046b0299d8ddf2087fd8a7f6ae0b7f6b006b0299d8ddf2087fd8a7f6ae0b7f6d02010303000100",
      silent_on_trap, my_principal);
  // Use larger type table of icpp
  mockIC.run_test(
      "roundtrip_vec_variant 2", roundtrip_vec_variant,
      "4449444c046b0299d8ddf2087fd8a7f6ae0b7f6b006b0299d8ddf2087fd8a7f6ae0b7f6d02010303000100",
      "4449444c046b0299d8ddf2087fd8a7f6ae0b7f6b006b0299d8ddf2087fd8a7f6ae0b7f6d02010303000100",
      silent_on_trap, my_principal);

  // vec record { "principal": text; role: variant { AdminQuery; AdminUpdate }; assignedBy: text; assignedAt: nat64; note: text }
  // Tests the complex type pattern used in llama_cpp_canister's getAdminRoles
  // Note: The output type table is larger than the input because VecRecord re-encodes
  // each record with its own variant type table entries, but values are semantically equivalent
  mockIC.run_test(
      "roundtrip_vec_record_variant", roundtrip_vec_record_variant,
      "4449444c036d016c05ae9db1900171e1e194b40278c5e394b40271f2afa8c80471f6d6bbdd04026b0299d8ddf2087fd8a7f6ae0b7f010002076162632d313233d2029649000000000a636f6e74726f6c6c65720474657374000778797a2d373839ea16b04c020000000b636f6e74726f6c6c65723205746573743201",
      "4449444c0f6b0299d8ddf2087fd8a7f6ae0b7f6c05ae9db1900102e1e194b40206c5e394b40202f2afa8c80402f6d6bbdd04056d716b006b0299d8ddf2087fd8a7f6ae0b7f6d046d786c006c05ae9db1900171e1e194b40278c5e394b40271f2afa8c80471f6d6bbdd04096b0299d8ddf2087fd8a7f6ae0b7f6d086c05ae9db1900171e1e194b40278c5e394b40271f2afa8c80471f6d6bbdd040c6b0299d8ddf2087fd8a7f6ae0b7f6c05ae9db1900171e1e194b40278c5e394b40271f2afa8c80471f6d6bbdd040e6b0299d8ddf2087fd8a7f6ae0b7f010a02076162632d313233d2029649000000000a636f6e74726f6c6c65720474657374000778797a2d373839ea16b04c020000000b636f6e74726f6c6c65723205746573743201",
      silent_on_trap, my_principal);

  // '(vec { true : bool; false : bool }, vec { 101 : nat; 102 : nat; 103 : nat }, vec { 101 : nat8; 102 : nat8; 103 : nat8 }, vec { 101 : nat16; 102 : nat16; 103 : nat16 }, vec { 101 : nat32; 102 : nat32; 103 : nat32 }, vec { 101 : nat64; 102 : nat64; 103 : nat64 }, vec { -101 : int; -102 : int; -103 : int }, vec { -101 : int8; -102 : int8; -103 : int8 }, vec { -101 : int16; -102 : int16; -103 : int16 }, vec { -101 : int32; -102 : int32; -103 : int32 }, vec { -101 : int64; -102 : int64; -103 : int64 }, vec { -1.01 : float32; -1.02 : float32; -1.03 : float32 }, vec { -1.01 : float64; -1.02 : float64; -1.03 : float64 }, vec { "Hello 101" : text; "Hello 102" : text; "Hello 103" : text }, vec { principal "2ibo7-dia"; principal "w3gef-eqbai"; principal "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae"})'
  // -> same
  mockIC.run_test(
      "roundtrip_vec_all", roundtrip_vec_all,
      "4449444c0f6d7e6d7d6d7b6d7a6d796d786d7c6d776d766d756d746d736d726d716d680f000102030405060708090a0b0c0d0e0201000365666703656667036500660067000365000000660000006700000003650000000000000066000000000000006700000000000000039b7f9a7f997f039b9a99039bff9aff99ff039bffffff9affffff99ffffff039bffffffffffffff9affffffffffffff99ffffffffffffff03ae4781bf5c8f82bf0ad783bf03295c8fc2f528f0bf52b81e85eb51f0bf7b14ae47e17af0bf030948656c6c6f203130310948656c6c6f203130320948656c6c6f203130330301010001020102011d779590d2cd339802981dfd935d9a3dbb085cafe6ad19b87229a016d602",
      "4449444c0f6d7e6d7d6d7b6d7a6d796d786d7c6d776d766d756d746d736d726d716d680f000102030405060708090a0b0c0d0e0201000365666703656667036500660067000365000000660000006700000003650000000000000066000000000000006700000000000000039b7f9a7f997f039b9a99039bff9aff99ff039bffffff9affffff99ffffff039bffffffffffffff9affffffffffffff99ffffffffffffff03ae4781bf5c8f82bf0ad783bf03295c8fc2f528f0bf52b81e85eb51f0bf7b14ae47e17af0bf030948656c6c6f203130310948656c6c6f203130320948656c6c6f203130330301010001020102011d779590d2cd339802981dfd935d9a3dbb085cafe6ad19b87229a016d602",
      silent_on_trap, my_principal);

  // ----------------------------------------------------------------------------------
  // Opt:
  // (-) An expected Opt can be on the wire as either a value or as (null)
  // (-) An expected Opt can be absent from the wire, which must be treated as (null)
  //
  // '(opt (101 : nat))' -> same
  mockIC.run_test("roundtrip_opt_nat", roundtrip_opt_nat,
                  "4449444c016e7d01000165", "4449444c016e7d01000165",
                  silent_on_trap, my_principal);

  // '(opt (101 : nat), 102 : nat)' -> '(102 : nat)'
  mockIC.run_test("roundtrip_opt_nat_nat 1", roundtrip_opt_nat_nat,
                  "4449444c016e7d02007d016566", "4449444c00017d66",
                  silent_on_trap, my_principal);

  // '(opt (null : null), 102 : nat)' -> '(102 : nat)'
  mockIC.run_test("roundtrip_opt_nat_nat 2", roundtrip_opt_nat_nat,
                  "4449444c016e7f02007d0166", "4449444c00017d66",
                  silent_on_trap, my_principal);

  // '(102 : nat)' -> '(102 : nat)'
  mockIC.run_test("roundtrip_opt_nat_nat 3", roundtrip_opt_nat_nat,
                  "4449444c00017d66", "4449444c00017d66", silent_on_trap,
                  my_principal);

  // '(null)' -> The raw bytes returned represent an opt : nat without a value.
  //             There is no IDL representation for this.
  //             didc decodes it to '(null)'
  mockIC.run_test("roundtrip_opt_nat_no_value", roundtrip_opt_nat_no_value,
                  "4449444c00017f", "4449444c016e7d010000", silent_on_trap,
                  my_principal);
  mockIC.run_test("roundtrip_opt_nat_no_value", roundtrip_opt_nat_no_value,
                  "4449444c016e7d010000", "4449444c016e7d010000",
                  silent_on_trap, my_principal);
  // '()' ->
  mockIC.run_test("roundtrip_opt_nat_no_value", roundtrip_opt_nat_no_value,
                  "4449444c0000", "4449444c016e7d010000", silent_on_trap,
                  my_principal);

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

  // '(opt ( record {field1 = "hello" : text; field2 = 16 : nat16} ))' -> same
  mockIC.run_test(
      "roundtrip_opt_record", roundtrip_opt_record,
      "4449444c026e016c02b79cba840871b89cba84087a0100010568656c6c6f1000",
      "4449444c036c02b79cba840871b89cba84087a6c006e000102010568656c6c6f1000",
      silent_on_trap, my_principal);

  // ------------------------------------------------------------------------
  // Opt Vec
  //
  // '(opt (vec { 101 : nat16; 102 : nat16; 103 : nat16 }))'
  // -> same
  mockIC.run_test("roundtrip_opt_vec_nat16", roundtrip_opt_vec_nat16,
                  "4449444c026e016d7a01000103650066006700",
                  "4449444c026d7a6e0001010103650066006700", silent_on_trap,
                  my_principal);
  mockIC.run_test("roundtrip_opt_vec_nat16 2", roundtrip_opt_vec_nat16,
                  "4449444c026d7a6e0001010103650066006700",
                  "4449444c026d7a6e0001010103650066006700", silent_on_trap,
                  my_principal);

  // '(opt (vec { record {field1 = "hello1" : text; field2 = "bye1" : text}; record {field1 = "hello2" : text; field2 = "bye2" : text}; record {field1 = "hello3" : text; field3 = "bye3" : text} }))'
  // -> same
  mockIC.run_test(
      "roundtrip_opt_vec_record", roundtrip_opt_vec_record,
      "4449444c036e016d026c02b79cba840871b89cba840871010001030668656c6c6f3104627965310668656c6c6f3204627965320668656c6c6f330462796533",
      "4449444c096c02b79cba840801b89cba8408016d716c006c02b79cba840871b89cba8408716d036c02b79cba840871b89cba8408716c02b79cba840871b89cba8408716c02b79cba840871b89cba8408716e04010801030668656c6c6f3104627965310668656c6c6f3204627965320668656c6c6f330462796533",
      silent_on_trap, my_principal);
  mockIC.run_test(
      "roundtrip_opt_vec_record 2", roundtrip_opt_vec_record,
      "4449444c096c02b79cba840801b89cba8408016d716c006c02b79cba840871b89cba8408716d036c02b79cba840871b89cba8408716c02b79cba840871b89cba8408716c02b79cba840871b89cba8408716e04010801030668656c6c6f3104627965310668656c6c6f3204627965320668656c6c6f330462796533",
      "4449444c096c02b79cba840801b89cba8408016d716c006c02b79cba840871b89cba8408716d036c02b79cba840871b89cba8408716c02b79cba840871b89cba8408716c02b79cba840871b89cba8408716e04010801030668656c6c6f3104627965310668656c6c6f3204627965320668656c6c6f330462796533",
      silent_on_trap, my_principal);

  // TODO: opt_vec_XXX      create a test for every vec_XXX type !!!

  // '()' -> '()'
  mockIC.run_test("roundtrip_opt_record_no_value 1",
                  roundtrip_opt_record_no_value, "4449444c0000", "4449444c0000",
                  silent_on_trap, my_principal);

  // '(null)' -> '()'
  mockIC.run_test("roundtrip_opt_record_no_value 2",
                  roundtrip_opt_record_no_value, "4449444c00017f",
                  "4449444c0000", silent_on_trap, my_principal);

  // '(opt ( null : null ))' -> ()
  mockIC.run_test("roundtrip_opt_record_no_value 3",
                  roundtrip_opt_record_no_value, "4449444c016e7f010001",
                  "4449444c0000", silent_on_trap, my_principal);

  // '(opt ( variant { Ok } ))' -> same
  mockIC.run_test("roundtrip_opt_variant_ok (1)", roundtrip_opt_variant_ok,
                  "4449444c026e016b01bc8a017f01000100", // (1)
                  "4449444c036b01bc8a017f6b006e0001020100", silent_on_trap,
                  my_principal);

  // '()' -> '()'
  mockIC.run_test("roundtrip_opt_variant_ok_no_value 1",
                  roundtrip_opt_variant_ok_no_value, "4449444c0000",
                  "4449444c0000", silent_on_trap, my_principal);

  // '(null)' -> '()'
  mockIC.run_test("roundtrip_opt_variant_ok_no_value 2",
                  roundtrip_opt_variant_ok_no_value, "4449444c00017f",
                  "4449444c0000", silent_on_trap, my_principal);

  // '(opt ( null : null ))' -> ()
  mockIC.run_test("roundtrip_opt_variant_ok_no_value 3",
                  roundtrip_opt_variant_ok_no_value, "4449444c016e7f010001",
                  "4449444c0000", silent_on_trap, my_principal);

  // '(opt ( variant { Err = "Error" : text} ))' -> same
  mockIC.run_test("roundtrip_opt_variant_err (1)", roundtrip_opt_variant_err,
                  "4449444c026e016b01c5fed2017101000100054572726f72", // (1)
                  "4449444c036b01c5fed201716b006e0001020100054572726f72",
                  silent_on_trap, my_principal);

  // '()' -> '()'
  mockIC.run_test("roundtrip_opt_variant_err_no_value 1",
                  roundtrip_opt_variant_err_no_value, "4449444c0000",
                  "4449444c0000", silent_on_trap, my_principal);

  // '(null)' -> '()'
  mockIC.run_test("roundtrip_opt_variant_err_no_value 2",
                  roundtrip_opt_variant_err_no_value, "4449444c00017f",
                  "4449444c0000", silent_on_trap, my_principal);

  // '(opt ( null : null ))' -> ()
  mockIC.run_test("roundtrip_opt_variant_err_no_value 3",
                  roundtrip_opt_variant_err_no_value, "4449444c016e7f010001",
                  "4449444c0000", silent_on_trap, my_principal);

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

  // ----------------------------------------------------------------------------------
  // Record
  //
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

  // '(record {"key1"        = record { "key2"        = record {"key3"         = "value1" : text;}}})'
  // '(record {1_191_633_330 = record { 1_191_633_331 = record { 1_191_633_332 = "value1" };
  // -> same
  // Candid with only used type tables, as created by didc, with type tables (record1,2,3)
  mockIC.run_test(
      "roundtrip_record_record_record 1", roundtrip_record_record_record,
      "4449444c036c01b2c39bb804016c01b3c39bb804026c01b4c39bb8047101000676616c756531",
      "4449444c036c01b4c39bb804716c01b3c39bb804006c01b2c39bb8040101020676616c756531",
      silent_on_trap, my_principal);
  // Candid with only used type tables, as created by icpp-pro, with type tables flipped (record3,2,1)
  mockIC.run_test(
      "roundtrip_record_record_record 2", roundtrip_record_record_record,
      "4449444c036c01b4c39bb804716c01b3c39bb804006c01b2c39bb8040101020676616c756531",
      "4449444c036c01b4c39bb804716c01b3c39bb804006c01b2c39bb8040101020676616c756531",
      silent_on_trap, my_principal);
  // candid_in contains unused type tables, as created by older version of icpp. Must work too.
  mockIC.run_test(
      "roundtrip_record_record_record 3", roundtrip_record_record_record,
      "4449444c046c006c01b4c39bb804716c01b3c39bb804016c01b2c39bb8040201030676616c756531",
      "4449444c036c01b4c39bb804716c01b3c39bb804006c01b2c39bb8040101020676616c756531",
      silent_on_trap, my_principal);

  // '(record {"names" = vec { "H1N" : text; "H2N" : text}; "values" = vec { "H1V" : text; "H2V" : text};})'
  // '(record {835_838_106 = ...})'
  // -> same
  // Candid with only used type tables
  mockIC.run_test(
      "roundtrip_record_vec_text 1", roundtrip_record_vec_text,
      "4449444c026c02e287dcfd0401c89f92b409016d710100020348315603483256020348314e0348324e",
      "4449444c026c02e287dcfd0401c89f92b409016d710100020348315603483256020348314e0348324e",
      silent_on_trap, my_principal);
  // candid_in contains unused type table. Also need to be able to decode this.
  mockIC.run_test(
      "roundtrip_record_vec_text 2", roundtrip_record_vec_text,
      "4449444c036c006d716c02e287dcfd0401c89f92b409010102020348315603483256020348314e0348324e",
      "4449444c026c02e287dcfd0401c89f92b409016d710100020348315603483256020348314e0348324e",
      silent_on_trap, my_principal);

  // ------------------------------------------------------------------------
  // Record with Opt fields

  //
  // Test 1: Both fields are provided with actual values
  // from_wire:
  // '(1 : nat8, record {field3 = opt ("hello" : text); field1 = opt (16 : nat16)})'
  // to_wire:
  // -> '(record { 2_156_826_167 = opt (16 : nat16); 2_156_826_168 = opt "hello" })'
  mockIC.run_test(
      "roundtrip_record_opt 1", roundtrip_record_opt,
      "4449444c036c02b79cba840801b99cba8408026e7a6e71027b0001011000010568656c6c6f",
      "4449444c036c02b79cba840802b99cba8408016e716e7a0100011000010568656c6c6f",
      silent_on_trap, my_principal);

  // Test 2: Both fields have no value
  //      a: Both fields are provided as 'null'
  // from_wire:
  // '(2 : nat8, record {field3 = opt (null : null); field1 = opt (null : null)})'
  // to_wire:
  // -> '(record { 2_156_826_167 = opt (16 : nat16); 2_156_826_168 = opt "hello" })'
  mockIC.run_test(
      "roundtrip_record_opt 2a", roundtrip_record_opt,
      "4449444c026c02b79cba840801b99cba8408016e7f027b00020101",
      "4449444c036c02b79cba840802b99cba8408016e716e7a0100011000010568656c6c6f",
      silent_on_trap, my_principal);

  // Test 2: Both fields have no value
  //      b: Both fields are omitted
  // from_wire:
  // '(2 : nat8, record {})'
  // to_wire:
  // -> '(record { 2_156_826_167 = opt (16 : nat16); 2_156_826_168 = opt "hello" })'
  mockIC.run_test(
      "roundtrip_record_opt 2 b", roundtrip_record_opt,
      "4449444c016c00027b0002",
      "4449444c036c02b79cba840802b99cba8408016e716e7a0100011000010568656c6c6f",
      silent_on_trap, my_principal);

  // Test 3: Only field1 has a value
  //      a: field1 is provided with actual value, field3 is provided as null
  // from_wire:
  // '(3 : nat8, record {field3 = opt (null : null); field1 = opt (16 : nat16) })'
  // to_wire:
  // -> '(record { 2_156_826_167 = opt (16 : nat16); 2_156_826_168 = opt "hello" })'
  mockIC.run_test(
      "roundtrip_record_opt 3a", roundtrip_record_opt,
      "4449444c036c02b79cba840801b99cba8408026e7a6e7f027b000301100001",
      "4449444c036c02b79cba840802b99cba8408016e716e7a0100011000010568656c6c6f",
      silent_on_trap, my_principal);

  // Test 3: Only field1 has a value
  //      b: field1 is provided with actual value, field3 is omitted
  // from_wire:
  // '(3 : nat8, record {field1 = opt (16 : nat16) })'
  // to_wire:
  // -> '(record { 2_156_826_167 = opt (16 : nat16); 2_156_826_168 = opt "hello" })'
  mockIC.run_test(
      "roundtrip_record_opt 3b", roundtrip_record_opt,
      "4449444c026c01b79cba8408016e7a027b0003011000",
      "4449444c036c02b79cba840802b99cba8408016e716e7a0100011000010568656c6c6f",
      silent_on_trap, my_principal);

  // Test 4: Only field3 has a value
  //      a: field1 is provided as null, field3 is provided with actual value,
  // from_wire:
  // '(4 : nat8, record {field3 = opt ("hello" : text);field1 = opt (null : null)})'
  // to_wire:
  // -> '(record { 2_156_826_167 = opt (16 : nat16); 2_156_826_168 = opt "hello" })'
  mockIC.run_test(
      "roundtrip_record_opt 4a", roundtrip_record_opt,
      "4449444c036c02b79cba840801b99cba8408026e7f6e71027b000401010568656c6c6f",
      "4449444c036c02b79cba840802b99cba8408016e716e7a0100011000010568656c6c6f",
      silent_on_trap, my_principal);

  // Test 4: Only field3 has a value
  //      b: field1 is omitted, field3 is provided with actual value
  // from_wire:
  // '(4 : nat8, record {field3 = opt ("hello" : text);})'
  // to_wire:
  // -> '(record { 2_156_826_167 = opt (16 : nat16); 2_156_826_168 = opt "hello" })'
  mockIC.run_test(
      "roundtrip_record_opt 4b", roundtrip_record_opt,
      "4449444c026c01b99cba8408016e71027b0004010568656c6c6f",
      "4449444c036c02b79cba840802b99cba8408016e716e7a0100011000010568656c6c6f",
      silent_on_trap, my_principal);

  // ------------------------------------------------------------------------
  // Record with Opt Variant fields

  //
  // Test 1: Both fields are provided with actual values
  // from_wire:
  // '(1 : nat8, record {field3 = opt (variant { Err = "Error" : text}); field1 = opt (variant { Ok })})'
  // to_wire:
  // -> '(record {field3 = opt (variant { Err = "Error" : text}); field1 = opt (variant { Ok })})'
  mockIC.run_test(
      "roundtrip_record_opt_variant 1", roundtrip_record_opt_variant,
      "4449444c056c02b79cba840801b99cba8408036e026b01bc8a017f6e046b01c5fed20171027b000101000100054572726f72",
      "4449444c066c02b79cba840805b99cba8408036b01c5fed201716b006e016b01bc8a017f6e04010001000100054572726f72",
      silent_on_trap, my_principal);

  // Test 2: Both fields have no value
  //      a: Both fields are provided as 'null'
  // from_wire:
  // '(2 : nat8, record {field3 = opt (null : null); field1 = opt (null : null)})'
  // to_wire:
  // -> '(record {field3 = opt (variant { Err = "Error" : text}); field1 = opt (variant { Ok })})'
  mockIC.run_test(
      "roundtrip_record_opt_variant 2a", roundtrip_record_opt_variant,
      "4449444c026c02b79cba840801b99cba8408016e7f027b00020101",
      "4449444c066c02b79cba840805b99cba8408036b01c5fed201716b006e016b01bc8a017f6e04010001000100054572726f72",
      silent_on_trap, my_principal);

  // Test 2: Both fields have no value
  //      b: Both fields are omitted
  // from_wire:
  // '(2 : nat8, record {})'
  // to_wire:
  // -> '(record {field3 = opt (variant { Err = "Error" : text}); field1 = opt (variant { Ok })})'
  mockIC.run_test(
      "roundtrip_record_opt_variant 2 b", roundtrip_record_opt_variant,
      "4449444c016c00027b0002",
      "4449444c066c02b79cba840805b99cba8408036b01c5fed201716b006e016b01bc8a017f6e04010001000100054572726f72",
      silent_on_trap, my_principal);

  // Test 3: Only field1 has a value
  //      a: field1 is provided with actual value, field3 is provided as null
  // from_wire:
  // '(3 : nat8, record {field3 = opt (null : null); field1 = opt (variant { Ok }) })'
  // to_wire:
  // -> '(record {field3 = opt (variant { Err = "Error" : text}); field1 = opt (variant { Ok })})'
  mockIC.run_test(
      "roundtrip_record_opt_variant 3a", roundtrip_record_opt_variant,
      "4449444c046c02b79cba840801b99cba8408036e026b01bc8a017f6e7f027b0003010001",
      "4449444c066c02b79cba840805b99cba8408036b01c5fed201716b006e016b01bc8a017f6e04010001000100054572726f72",
      silent_on_trap, my_principal);

  // Test 3: Only field1 has a value
  //      b: field1 is provided with actual value, field3 is omitted
  // from_wire:
  // '(3 : nat8, record {field1 = opt (variant { Ok }) })'
  // to_wire:
  // -> '(record {field3 = opt (variant { Err = "Error" : text}); field1 = opt (variant { Ok })})'
  mockIC.run_test(
      "roundtrip_record_opt_variant 3b", roundtrip_record_opt_variant,
      "4449444c036c01b79cba8408016e026b01bc8a017f027b00030100",
      "4449444c066c02b79cba840805b99cba8408036b01c5fed201716b006e016b01bc8a017f6e04010001000100054572726f72",
      silent_on_trap, my_principal);

  // Test 4: Only field3 has a value
  //      a: field1 is provided as null, field3 is provided with actual value,
  // from_wire:
  // '(4 : nat8, record {field3 = opt (variant { Err = "Error" : text});field1 = opt (null : null)})'
  // to_wire:
  // -> '(record {field3 = opt (variant { Err = "Error" : text}); field1 = opt (variant { Ok })})'
  mockIC.run_test(
      "roundtrip_record_opt_variant 4a", roundtrip_record_opt_variant,
      "4449444c046c02b79cba840801b99cba8408026e7f6e036b01c5fed20171027b0004010100054572726f72",
      "4449444c066c02b79cba840805b99cba8408036b01c5fed201716b006e016b01bc8a017f6e04010001000100054572726f72",
      silent_on_trap, my_principal);

  // Test 4: Only field3 has a value
  //      b: field1 is omitted, field3 is provided with actual value
  // from_wire:
  // '(4 : nat8, record {field3 = opt (variant { Err = "Error" : text});})'
  // to_wire:
  // -> '(record {field3 = opt (variant { Err = "Error" : text}); field1 = opt (variant { Ok })})'
  mockIC.run_test(
      "roundtrip_record_opt_variant 4b", roundtrip_record_opt_variant,
      "4449444c036c01b99cba8408016e026b01c5fed20171027b00040100054572726f72",
      "4449444c066c02b79cba840805b99cba8408036b01c5fed201716b006e016b01bc8a017f6e04010001000100054572726f72",
      silent_on_trap, my_principal);

  // ------------------------------------------------------------------------
  // Record with Vec Record field (headers of http_request)
  // '(record { headers = vec { record {name = "H1N" : text; value = "H1V"}; record {name = "H2N" : text; value = "H2V"}; record {name = "H3N" : text; value = "H3V"}; } })'
  // -> same
  // Use compact type table of didc
  mockIC.run_test(
      "roundtrip_record_vec_record", roundtrip_record_vec_record,
      "4449444c036c01c6a4a19806016d026c02f1fee18d0371cbe4fdc70471010003034831560348314e034832560348324e034833560348334e",
      "4449444c096c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c01c6a4a1980604010803034831560348314e034832560348324e034833560348334e",
      silent_on_trap, my_principal);
  // Use larger type table of icpp
  mockIC.run_test(
      "roundtrip_record_vec_record 2", roundtrip_record_vec_record,
      "4449444c096c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c01c6a4a1980604010803034831560348314e034832560348324e034833560348334e",
      "4449444c096c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c01c6a4a1980604010803034831560348314e034832560348324e034833560348334e",
      silent_on_trap, my_principal);

  // Record with Vec Record field (headers of http_request)
  // '(record { headers = vec { record { "host"; "5ugrv-zqaaa-aaaag-acfna-cai.raw.icp0.io" };record { "x-real-ip"; "21.98.241.245" }; record { "x-forwarded-for"; "21.98.241.245" }; record { "x-forwarded-proto"; "https" }; record { "x-request-id"; "d9b41db4-4e97-8276-14f1-6db7d007b57d" }; record { "user-agent"; "curl/7.81.0" }; record { "accept"; "*/*" }; }; })'
  // -> same
  // Use compact type table of didc
  mockIC.run_test(
      "roundtrip_http_headers", roundtrip_http_headers,
      "4449444c036c01c6a4a19806016d026c020071017101000704686f73742735756772762d7a716161612d61616161672d6163666e612d6361692e7261772e696370302e696f09782d7265616c2d69700d32312e39382e3234312e3234350f782d666f727761726465642d666f720d32312e39382e3234312e32343511782d666f727761726465642d70726f746f0568747470730c782d726571756573742d69642464396234316462342d346539372d383237362d313466312d3664623764303037623537640a757365722d6167656e740b6375726c2f372e38312e3006616363657074032a2f2a",
      "4449444c0d6c02000101016d716c006c02007101716d036c02007101716c02007101716c02007101716c02007101716c02007101716c02007101716c02007101716c01c6a4a1980604010c0704686f73742735756772762d7a716161612d61616161672d6163666e612d6361692e7261772e696370302e696f09782d7265616c2d69700d32312e39382e3234312e3234350f782d666f727761726465642d666f720d32312e39382e3234312e32343511782d666f727761726465642d70726f746f0568747470730c782d726571756573742d69642464396234316462342d346539372d383237362d313466312d3664623764303037623537640a757365722d6167656e740b6375726c2f372e38312e3006616363657074032a2f2a",
      silent_on_trap, my_principal);

  // ----------------------------------------------------------------------------------
  // Opt Record Variant
  // '(opt ( record {field1 = "hello" : text; field2 = variant {fieldv = 16 : int}; field3 = "bye" : text; } ))' -> same
  mockIC.run_test(
      "roundtrip_opt_record_variant", roundtrip_opt_record_variant,
      "4449444c036e016c03b79cba840871b89cba840802b99cba8408716b01fc9cba84087c0100010568656c6c6f001003627965",
      "4449444c046b01fc9cba84087c6c03b79cba840871b89cba840800b99cba8408716c006e010103010568656c6c6f001003627965",
      silent_on_trap, my_principal);

  // ----------------------------------------------------------------------------------
  // Variant
  //
  // Candid's Variant has two options for the Type Table
  // (1) only include the type table of the variant's value
  // (2) include the type table of all the variant's cases

  // '(variant { Ok })' -> same
  mockIC.run_test("roundtrip_variant_ok (1)", roundtrip_variant_ok,
                  "4449444c016b01bc8a017f010000", // (1)
                  "4449444c016b01bc8a017f010000", silent_on_trap, my_principal);
  mockIC.run_test("roundtrip_variant_ok (2)", roundtrip_variant_ok,
                  "4449444c016b02bc8a017fc5fed20171010000", // (2)
                  "4449444c016b01bc8a017f010000", silent_on_trap, my_principal);

  // '(variant { Err = "Error" : text})' -> same
  mockIC.run_test("roundtrip_variant_err (1)", roundtrip_variant_err,
                  "4449444c016b01c5fed20171010000054572726f72", // (1)
                  "4449444c016b01c5fed20171010000054572726f72", silent_on_trap,
                  my_principal);
  mockIC.run_test("roundtrip_variant_err (2)", roundtrip_variant_err,
                  "4449444c016b02bc8a0171c5fed20171010001054572726f72", // (2)
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
      "roundtrip_variant 1", roundtrip_variant,
      "4449444c046b01bc8a017f6b01c5fed2017a6b01bc8a01716b01c5fed2017106000102010203000094010008416c6c20676f6f6400f4010008416c6c20676f6f6400054572726f72",
      "4449444c066b02bc8a017fc5fed2017a6b02bc8a017fc5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed201716b02bc8a0171c5fed2017106000102030405000194010008416c6c20676f6f6401f4010008416c6c20676f6f6401054572726f72",
      silent_on_trap, my_principal);
  mockIC.run_test(
      "roundtrip_variant 2", roundtrip_variant,
      "4449444c036b02bc8a017fc5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed2017106000001010202000194010008416c6c20676f6f6401f4010008416c6c20676f6f6401054572726f72",
      "4449444c066b02bc8a017fc5fed2017a6b02bc8a017fc5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed201716b02bc8a0171c5fed2017106000102030405000194010008416c6c20676f6f6401f4010008416c6c20676f6f6401054572726f72",
      silent_on_trap, my_principal);
  mockIC.run_test(
      "roundtrip_variant 3", roundtrip_variant,
      "4449444c066b02bc8a017fc5fed2017a6b02bc8a017fc5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed201716b02bc8a0171c5fed2017106000102030405000194010008416c6c20676f6f6401f4010008416c6c20676f6f6401054572726f72",
      "4449444c066b02bc8a017fc5fed2017a6b02bc8a017fc5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed2017a6b02bc8a0171c5fed201716b02bc8a0171c5fed2017106000102030405000194010008416c6c20676f6f6401f4010008416c6c20676f6f6401054572726f72",
      silent_on_trap, my_principal);

  // ------------------------------------------------------------------------
  // Variant with Opt field

  //
  // Test 1: Field is provided with a value
  // from_wire:
  // '(1 : nat8, variant {Err = opt ("Error" : text)})'
  // to_wire:
  // -> '(variant { 3_456_837 = opt "Error" })'
  mockIC.run_test("roundtrip_variant_opt 1", roundtrip_variant_opt,
                  "4449444c026b01c5fed201016e71027b00010001054572726f72",
                  "4449444c026b01c5fed201016e7101000001054572726f72",
                  silent_on_trap, my_principal);

  // Test 2: Field has no value
  //      a: Field is provided as 'null'
  // from_wire:
  // '(2 : nat8, variant {Err = opt (null : null)})'
  // to_wire:
  // -> '(variant { 3_456_837 = opt "Error" })'
  mockIC.run_test("roundtrip_variant_opt 2a", roundtrip_variant_opt,
                  "4449444c026b01c5fed201016e7f027b00020001",
                  "4449444c026b01c5fed201016e7101000001054572726f72",
                  silent_on_trap, my_principal);

  // ---------------------------------------------------------------------------------
  // Forward Compatibility Tests
  //
  // See: A Candid explainer: Opt is special
  //      https://www.joachim-breitner.de/blog/784-A_Candid_explainer__Opt_is_special
  //
  // Forward compatibility of service interfaces:
  // Interfaces can evolve without breaking old clients by using Opt:
  //  (-) Additional Opt args
  //  (-) Additional Opt fields in Record or Variant
  //
  //  -> These additional Opts are read & discarded during Candid deserialization
  //

  // '(opt (null: null), true, opt (null: null))' -> '(true)'
  mockIC.run_test("roundtrip_bool_true with additional Opt 1",
                  roundtrip_bool_true, "4449444c016e7f03007e00010101",
                  "4449444c00017e01", silent_on_trap, my_principal);

  // '(opt ("extra 1": text), true, opt (101: nat8))' -> '(true)'
  mockIC.run_test("roundtrip_bool_true with additional Opt 2",
                  roundtrip_bool_true,
                  "4449444c026e716e7b03007e01010765787472612031010165",
                  "4449444c00017e01", silent_on_trap, my_principal);

  // '(opt ( record {field2 = "f2" : text; field1 = 16 : nat16} ), true, opt (101: nat8))' -> '(true)'
  mockIC.run_test(
      "roundtrip_bool_true with additional Opt 3", roundtrip_bool_true,
      "4449444c036e016c02b79cba84087ab89cba8408716e7b03007e02011000026632010165",
      "4449444c00017e01", silent_on_trap, my_principal);

  // '(opt ( variant { Ok } ), true, opt (variant { Err = "Error" : text}))' -> '(true)'
  mockIC.run_test(
      "roundtrip_bool_true with additional Opt 4", roundtrip_bool_true,
      "4449444c046e016b01bc8a017f6e036b01c5fed2017103007e020100010100054572726f72",
      "4449444c00017e01", silent_on_trap, my_principal);

  // '(opt (variant { Err = "Error" : text}), true, opt ( variant { Ok } ))' -> '(true)'
  mockIC.run_test(
      "roundtrip_bool_true with additional Opt 5", roundtrip_bool_true,
      "4449444c046e016b01c5fed201716e036b01bc8a017f03007e020100054572726f72010100",
      "4449444c00017e01", silent_on_trap, my_principal);

  // '(opt ( record {field1 = "hello" : text} ), opt (variant { Err = "Error" : text}), true, opt ( variant { Ok } ), opt ( record {field2 = "bye" : text} ))'
  // -> '(true)'
  mockIC.run_test(
      "roundtrip_bool_true with additional Opt 6", roundtrip_bool_true,
      "4449444c086e016c01b79cba8408716e036b01c5fed201716e056b01bc8a017f6e076c01b89cba8408710500027e0406010568656c6c6f0100054572726f720101000103627965",
      "4449444c00017e01", silent_on_trap, my_principal);

  // '(opt (record {opt (variant {field1 = "hello" : text})}), opt (variant { Err = "Error" : text}), true, opt ( variant { Ok } ), opt (record {opt (variant {field2 = "bye" : text})}))'
  // -> '(true)'
  mockIC.run_test(
      "roundtrip_bool_true with additional Opt 7", roundtrip_bool_true,
      "4449444c0c6e016c0100026e036b01b79cba8408716e056b01c5fed201716e076b01bc8a017f6e096c01000a6e0b6b01b89cba8408710500047e06080101000568656c6c6f0100054572726f7201010001010003627965",
      "4449444c00017e01", silent_on_trap, my_principal);

  // Test complex but valid opt variant with opt variant field
  // '(opt (variant { Err = opt (variant { Err = "Error" : text}) }), true, opt ( variant { Ok } ), opt (variant { Err = opt (variant { Err = "Error" : text}) }))'
  // -> '(true)'
  mockIC.run_test(
      "roundtrip_bool_true with additional Opt 8", roundtrip_bool_true,
      "4449444c066e016b01c5fed201026e036b01c5fed201716e056b01bc8a017f04007e040001000100054572726f7201010001000100054572726f72",
      "4449444c00017e01", silent_on_trap, my_principal);

  // '(opt (record {opt (variant { Err = opt (variant { Err = "Error" : text}) })}), opt (variant { Err = "Error" : text}), true, opt ( variant { Ok } ), opt (record {opt (variant { Err = opt (variant { Err = "Error" : text}) })}))'
  // -> '(true)'
  mockIC.run_test(
      "roundtrip_bool_true with additional Opt 9", roundtrip_bool_true,
      "4449444c086e016c0100026e036b01c5fed201046e056b01c5fed201716e076b01bc8a017f0500047e06000101000100054572726f720100054572726f720101000101000100054572726f72",
      "4449444c00017e01", silent_on_trap, my_principal);

  // '(opt (vec { 101 : nat16; 102 : nat16; 103 : nat16 }), true, opt (vec { 101 : nat16; 102 : nat16; 103 : nat16 }))' -> '(true)'
  mockIC.run_test(
      "roundtrip_bool_true with additional Opt 10", roundtrip_bool_true,
      "4449444c026e016d7a03007e000103650066006700010103650066006700",
      "4449444c00017e01", silent_on_trap, my_principal);

  // '(opt (vec { record {field1 = "hello1" : text; field2 = "bye1" : text}; record {field1 = "hello2" : text; field2 = "bye2" : text}; record {field1 = "hello2" : text; field3 = "bye3" : text} }), true, opt (vec { record {field1 = "hello1" : text; field2 = "bye1" : text}; record {field1 = "hello2" : text; field2 = "bye2" : text}; record {field1 = "hello2" : text; field3 = "bye3" : text} }))'
  // -> '(true)'
  mockIC.run_test(
      "roundtrip_bool_true with additional Opt 11", roundtrip_bool_true,
      "4449444c036e016d026c02b79cba840871b89cba84087103007e0001030668656c6c6f3104627965310668656c6c6f3204627965320668656c6c6f3204627965330101030668656c6c6f3104627965310668656c6c6f3204627965320668656c6c6f320462796533",
      "4449444c00017e01", silent_on_trap, my_principal);

  // '(opt (vec { record {field1 = "hello1" : text; field2 = variant {fieldv = 1 : int}; field3 = "bye1" : text}; record {field1 = "hello2" : text; field2 = variant {fieldv = 2 : int}; field3 = "bye2" : text}; record {field1 = "hello2" : text; field2 = variant {fieldv = 2 : int}; field3 = "bye2" : text} }), true, opt (vec { record {field1 = "hello1" : text; field2 = variant {fieldv = 1 : int}; field3 = "bye1" : text}; record {field1 = "hello2" : text; field2 = variant {fieldv = 2 : int}; field3 = "bye2" : text}; record {field1 = "hello2" : text; field2 = variant {fieldv = 2 : int}; field3 = "bye2" : text} }))'
  // -> '(true)'
  mockIC.run_test(
      "roundtrip_bool_true with additional Opt 12", roundtrip_bool_true,
      "4449444c046e016d026c03b79cba840871b89cba840803b99cba8408716b01fc9cba84087c03007e0001030668656c6c6f31000104627965310668656c6c6f32000204627965320668656c6c6f32000204627965320101030668656c6c6f31000104627965310668656c6c6f32000204627965320668656c6c6f3200020462796532",
      "4449444c00017e01", silent_on_trap, my_principal);

  // Test 1: Both fields are provided with actual values
  //  fwc a: Additional Opt fields at the beginning, in the middle and the end
  // from_wire:
  // '(1 : nat8, record {field4 = opt ("end opt" : text); field3 = opt ("hello" : text); field2 = opt ("middle opt" : text); field1 = opt (16 : nat16); field0 = opt ("start opt" : text);})'
  // to_wire:
  // -> '(record { 2_156_826_167 = opt (16 : nat16); 2_156_826_168 = opt "hello" })'
  mockIC.run_test(
      "roundtrip_record_opt 1 fwc a", roundtrip_record_opt,
      "4449444c036c05b69cba840801b79cba840802b89cba840801b99cba840801ba9cba8408016e716e7a027b000101097374617274206f7074011000010a6d6964646c65206f7074010568656c6c6f0107656e64206f7074",
      "4449444c036c02b79cba840802b99cba8408016e716e7a0100011000010568656c6c6f",
      silent_on_trap, my_principal);

  // Test 1: Both fields are provided with actual values
  //  fwc b: Additional Opt null fields at the beginning, in the middle and the end
  // from_wire:
  // '(1 : nat8, record {field4 = opt (null : null); field3 = opt ("hello" : text); field2 = opt (null : null); field1 = opt (16 : nat16); field0 = opt (null : null);})'
  // to_wire:
  // -> '(record { 2_156_826_167 = opt (16 : nat16); 2_156_826_168 = opt "f2" })'
  mockIC.run_test(
      "roundtrip_record_opt 1 fwc b", roundtrip_record_opt,
      "4449444c046c05b69cba840801b79cba840802b89cba840801b99cba840803ba9cba8408016e7f6e7a6e71027b00010101100001010568656c6c6f01",
      "4449444c036c02b79cba840802b99cba8408016e716e7a0100011000010568656c6c6f",
      silent_on_trap, my_principal);

  // vec record with additional Opt fields in the record
  // '(vec { record {val = opt ("begin opt" : text); 1000000000 = opt ("middle opt" : text); name_ = opt ("end opt" : text); name = "H1N" : text; value = "H1V"}; record {val = opt ("begin opt" : text); 1000000000 = opt ("middle opt" : text); name_ = opt ("end opt" : text); name = "H2N" : text; value = "H2V"}; record {val = opt ("begin opt" : text); 1000000000 = opt ("middle opt" : text); name_ = opt ("end opt" : text); name = "H3N" : text; value = "H3V"}; })'
  //   (
  //     vec {
  //         record {
  //         5_889_761 = opt "begin opt";
  //         834_174_833 = "H1V";
  //         1_000_000_000 = opt "middle opt";
  //         1_224_700_491 = "H1N";
  //         2_525_269_940 = opt "end opt";
  //         };
  //         record {
  //         5_889_761 = opt "begin opt";
  //         834_174_833 = "H2V";
  //         1_000_000_000 = opt "middle opt";
  //         1_224_700_491 = "H2N";
  //         2_525_269_940 = opt "end opt";
  //         };
  //         record {
  //         5_889_761 = opt "begin opt";
  //         834_174_833 = "H3V";
  //         1_000_000_000 = opt "middle opt";
  //         1_224_700_491 = "H3N";
  //         2_525_269_940 = opt "end opt";
  //         };
  //     },
  //   )
  // -> vec with records, but without the opt fields
  mockIC.run_test(
      "roundtrip_vec_record w opts", roundtrip_vec_record,
      "4449444c036d016c05e1bde70202f1fee18d03718094ebdc0302cbe4fdc70471b49f92b409026e710100030109626567696e206f707403483156010a6d6964646c65206f70740348314e0107656e64206f70740109626567696e206f707403483256010a6d6964646c65206f70740348324e0107656e64206f70740109626567696e206f707403483356010a6d6964646c65206f70740348334e0107656e64206f7074",
      "4449444c086c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc70471010403034831560348314e034832560348324e034833560348334e",
      silent_on_trap, my_principal);

  // vec record with additional Opt fields in the record
  // '(vec { record {val = opt (variant { Okbegin }); 1000000000 = opt (variant { Okmiddle }); name_ = opt (variant { Err = "End" : text}); name = "H1N" : text; value = "H1V"}; record {val = opt (variant { Okbegin }); 1000000000 = opt (variant { Okmiddle }); name_ = opt (variant { Err = "End" : text}); name = "H2N" : text; value = "H2V"}; record {val = opt (variant { Okbegin }); 1000000000 = opt (variant { Okmiddle }); name_ = opt (variant { Err = "End" : text}); name = "H3N" : text; value = "H3V"}; })'
  //   (
  //     vec {
  //         record {
  //         5_889_761 = opt variant { 1_302_670_189 };
  //         834_174_833 = "H1V";
  //         1_000_000_000 = opt variant { 1_416_146_321 };
  //         1_224_700_491 = "H1N";
  //         2_525_269_940 = opt variant { 3_456_837 = "End" };
  //         };
  //         record {
  //         5_889_761 = opt variant { 1_302_670_189 };
  //         834_174_833 = "H2V";
  //         1_000_000_000 = opt variant { 1_416_146_321 };
  //         1_224_700_491 = "H2N";
  //         2_525_269_940 = opt variant { 3_456_837 = "End" };
  //         };
  //         record {
  //         5_889_761 = opt variant { 1_302_670_189 };
  //         834_174_833 = "H3V";
  //         1_000_000_000 = opt variant { 1_416_146_321 };
  //         1_224_700_491 = "H3N";
  //         2_525_269_940 = opt variant { 3_456_837 = "End" };
  //         };
  //     },
  //     )
  // -> vec with records, but without the opt fields
  mockIC.run_test(
      "roundtrip_vec_record w opt variant", roundtrip_vec_record,
      "4449444c086d016c05e1bde70202f1fee18d03718094ebdc0304cbe4fdc70471b49f92b409066e036b01edd694ed047f6e056b0191dba2a3057f6e076b01c5fed2017101000301000348315601000348314e010003456e6401000348325601000348324e010003456e6401000348335601000348334e010003456e64",
      "4449444c086c02f1fee18d0301cbe4fdc704016d716c006c02f1fee18d0371cbe4fdc704716d036c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc704716c02f1fee18d0371cbe4fdc70471010403034831560348314e034832560348324e034833560348334e",
      silent_on_trap, my_principal);

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

  // Both fields are provided with actual values, with a new non-Opt at the beginning,which must trap
  // from_wire:
  // '(1 : nat8, record {field4 = opt (null : null); field3 = opt ("hello" : text); field2 = opt (null : null); field1 = opt (16 : nat16); field0 = "f0 non opt";})'
  mockIC.run_trap_test(
      "roundtrip_record_opt 1 trap a", roundtrip_record_opt,
      "4449444c046c05b69cba840871b79cba840801b89cba840802b99cba840803ba9cba8408026e7a6e7f6e71027b00010a6630206e6f6e206f707401100001010568656c6c6f01",
      silent_on_trap, my_principal);

  // Both fields are provided with actual values, with a new non-Opt in the middle,which must trap
  // from_wire:
  // '(1 : nat8, record {field4 = opt (null : null); field3 = opt ("hello" : text); field2 = "f2 non opt"; field1 = opt (16 : nat16); field0 = opt ("start opt" : text);})'
  // to_wire:
  // -> TRAP !
  mockIC.run_trap_test(
      "roundtrip_record_opt 1 trap b", roundtrip_record_opt,
      "4449444c046c05b69cba840801b79cba840802b89cba840871b99cba840801ba9cba8408036e716e7a6e7f027b000101097374617274206f70740110000a6632206e6f6e206f7074010568656c6c6f01",
      silent_on_trap, my_principal);

  // Verify that a variant traps if the variant's id (hash) on wire does not match expected
  // '(variant { Blah = "BlahBlah" : text})' goes in, but it expects a variant option "Err"
  mockIC.run_trap_test("roundtrip_variant_err (trap 1)", roundtrip_variant_err,
                       "4449444c016b01d1bac9df027101000008426c6168426c6168",
                       silent_on_trap);

  // Verify that a variant traps if the variant's id (hash) on wire is ok, but type does not match
  // '(variant { Err = 1 : nat16})' goes in, but it expects a variant option "Err" : text
  mockIC.run_trap_test("roundtrip_variant_err  (trap 1)", roundtrip_variant_err,
                       "4449444c016b01c5fed2017a0100000100", silent_on_trap);

  // ------------------------------------------------------------------------
  // Trap test for variant.
  // Field has a new label, which must trap, even if the field is defined as opt
  // (See forum: https://forum.dfinity.org/t/question-about-record-variant-with-opt-fields-in-light-of-forward-compatibility/24567/2?u=icpp)

  //  -> same
  // from_wire:
  // Expecting: '(variant { Ok  })'
  // Receiving: '(variant { New })'
  // to_wire:
  // -> trap
  mockIC.run_trap_test("roundtrip_variant_ok trap", roundtrip_variant_ok,
                       "4449444c016b01c090ee017f010000", silent_on_trap,
                       my_principal);
  // from_wire:
  // Expecting: '(2 : nat8, variant {Err = opt (---)})'
  // Receiving: '(2 : nat8, variant {New = opt (null : null)})'
  // to_wire:
  // -> trap
  mockIC.run_trap_test("roundtrip_variant_opt 2a trap 1", roundtrip_variant_opt,
                       "4449444c026b01c090ee01016e7f027b00020001",
                       silent_on_trap, my_principal);
  // from_wire:
  // Expecting: '(2 : nat8, variant {Err = opt (---)})'
  // Receiving: '(2 : nat8, variant {New})'
  // to_wire:
  // -> trap
  mockIC.run_trap_test("roundtrip_variant_opt 2a trap 2", roundtrip_variant_opt,
                       "4449444c016b01c090ee017f027b000200", silent_on_trap,
                       my_principal);

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
