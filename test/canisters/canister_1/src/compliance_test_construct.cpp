// https://github.com/dfinity/candid/blob/master/test/construct.test.did

#include "compliance_test_construct.h"

#include <string>

#include "ic_api.h"

void test_records() {
  IC_API::debug_print("   - " + std::string(__func__));
  // record
  {
    // assert blob "DIDL\01\6c\00\01\00" == "(record {})"                                : (record {}) "record: empty";
    // didc encode '(record {})'
    CandidTypeRecord r;
    if (CandidSerialize(r).assert_candid("4449444c016c000100", true))
      IC_API::trap(std::string(__func__) + ": 1");
    // TODO: roundtrip decoding check
  }
  {
    // assert blob "DIDL\01\6c\00\03\00\00\00" == "(record {}, record {}, record {})"    : (record {}, record {}, record {}) "record: multiple records";
    // didc encode '(record {}, record {}, record {})'
    // These alternative encodings are OK:
    // 4449444c036c006c006c0003000102 : using 3 entries in the type table, and reference the type table index
    // 4449444c016c0003000000         : compressed format, with 1 entry in the type table, referenced 3 times, because values are the same

    CandidTypeRecord r1;
    CandidTypeRecord r2;
    CandidTypeRecord r3;
    std::vector<CandidType> A;
    A.push_back(r1);
    A.push_back(r2);
    A.push_back(r3);
    if (CandidSerialize(A).assert_candid("4449444c016c0003000000", true))
      IC_API::trap(std::string(__func__) + ": 2");
    // TODO: roundtrip decoding check
    // We do not (yet) check if the byte stream can be compressed.
    // It seems counter effective actually to check on compression by default.
    // if (CandidSerialize(A).assert_candid("4449444c016c0003000000", true))
    //   IC_API::trap("2");
  }
  {
    // assert blob "DIDL\01\6c\01\01\7c\01\00\2a" == "(record { 1 = 42 })"               : (record {1:int}) "record";
    // didc encode '(record { 1 = 42 : int })'
    CandidTypeRecord r;
    r.append(1, CandidTypeInt(42));
    if (CandidSerialize(r).assert_candid("4449444c016c01017c01002a", true))
      IC_API::trap(std::string(__func__) + ": 3");
    // TODO: roundtrip decoding check
  }
  {
      // assert blob "DIDL\01\6c\01\01\7c\01\00\2a" == "(record { 1 = opt 42 })"           : (record {1:opt int}) "record: opt";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 4");
      // TODO: roundtrip decoding check
  } {
      // assert blob "DIDL\01\6c\01\01\7c\01\00\2a" == "(record { 1 = null })"             : (record {1:reserved}) "record: reserved";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 5");
      // TODO: roundtrip decoding check
  } {
      // assert blob "DIDL\01\6c\01\01\7c\01\00\2a" == "(record {})"                       : (record {}) "record: ignore fields";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 6");
      // TODO: roundtrip decoding check
  } {
      // assert "(record { whatever = 0 })"  == "(record {})"                              : (record {}) "record: ignore fields (textual)";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 7");
      // TODO: roundtrip decoding check
  } {
      // assert blob "DIDL\01\6c\01\01\7c\01\00\2a"                                       !: (record {2:int}) "record: missing field";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 8");
      // TODO: roundtrip decoding check
  } {
      // assert blob "DIDL\01\6c\01\01\7c\01\00\2a" == "(record { 2 = null })"             : (record {2:opt int}) "record: missing opt field";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 9");
      // TODO: roundtrip decoding check
  } {
      // assert blob "DIDL\01\6c\02\00\7c\01\7e\01\00\2a\01" == "(record {42; true})"      : (record {int; bool}) "record: tuple";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 10");
      // TODO: roundtrip decoding check
  } {
      // assert blob "DIDL\01\6c\02\00\7c\01\7e\01\00\2a\01" == "(record {1 = true})"      : (record {1:bool}) "record: ignore fields";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 11");
      // TODO: roundtrip decoding check
  } {
      // assert blob "DIDL\01\6c\02\00\7c\01\7e\01\00\2a\01"                              !: (record {bool; int}) "record: type mismatch";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 12");
      // TODO: roundtrip decoding check
  } {
      // assert blob "DIDL\01\6c\02\00\7c\00\7e\01\00\2a\01"                              !: (record {int; bool}) "record: duplicate fields";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 13");
      // TODO: roundtrip decoding check
  } {
    // assert blob "DIDL\01\6c\02\01\7c\00\7e\01\00\2a\01"                              !: (record {1:int; 0:bool}) "record: unsorted";
    // (A) encoding must sort it:
    //     didc encode '(record {1 = 42 : int; 0 = true : bool})'
    //     4449444c016c02007e017c0100012a
    CandidTypeRecord r;
    r.append(1, CandidTypeInt(42));
    r.append(0, CandidTypeBool(true));
    if (CandidSerialize(r).assert_candid("4449444c016c02007e017c0100012a",
                                         true))
      IC_API::trap(std::string(__func__) + ": 14");

    //
    // (B) decoding of unsorted record must give error
    //     didc decode 4449444c016c02017c007e01002a01
    //     Error: Cannot parse header 4449444c016c02017c007e01002a01
    //     Caused by:
    //       0: Invalid table entry 0: Record(Fields { len: 2, inner: [FieldType { id: 1, index: IndexType { index: -4 } }, FieldType { id: 0, index: IndexType { index: -2 } }] })
    //       1: field id 0 collision or not sorted
    // // TODO: roundtrip decoding check
  }
  {
      // assert blob "DIDL\01\6c\02\d3\e3\aa\02\7e\86\8e\b7\02\7c\01\00\01\2a"
      //    == "(record { foo = 42; bar = true})"                                         : (record {foo:int; bar:bool}) "record: named fields";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 15");
      // TODO: roundtrip decoding check
  } {
      // assert blob "DIDL\01\6c\01\cd\84\b0\05\7f\01\00" == "(record { \"☃\" = null })"   : (record { "☃":null }) "record: unicode field";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 16");
      // TODO: roundtrip decoding check
  } {
      // assert blob "DIDL\01\6c\01\80\e4\97\d0\12\7c\01\00\2a"                         !: (record {}) "record: field hash larger than u32";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 17");
      // TODO: roundtrip decoding check
  } {
      // assert blob "DIDL\04\6c\01\00\01\6c\01\00\02\6c\01\00\03\6c\00\01\00"
      //    == "(record { record { record { record {} } } })"                           : (record{record{record{record{}}}}) "record: nested";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 18");
      // TODO: roundtrip decoding check
  } {
      // assert blob "DIDL\01\6c\01\00\00\01\00"                                        !: (EmptyRecord) "record: empty recursion";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 19");
      // TODO: roundtrip decoding check
  } {
      // assert blob "DIDL\02\6d\01\6c\01\00\01\01\00\01"                               !: (vec EmptyRecord) "vec of empty records";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 20");
      // TODO: roundtrip decoding check
  } {
      // assert blob "DIDL\01\6c\02\00\7c\01\7e\01\00\2a"                               !: (record {int}) "record: value too short";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 21");
      // TODO: roundtrip decoding check
  } {
      // assert blob "DIDL\01\6c\02\00\7c\01\7e\01\00\2a\01\00"                         !: (record {int}) "record: value too long";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 22");
      // TODO: roundtrip decoding check
  } {
      // assert blob "DIDL\01\6c\02\00\7c\01\01\00\2a\01"                               !: (record {}) "record: type too short";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 23");
      // TODO: roundtrip decoding check
  } {
      // assert blob "DIDL\01\6c\02\00\7c\01\00\2a\01"                                  !: (record {}) "record: type too short";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 24");
      // TODO: roundtrip decoding check
  } {
      //assert blob "DIDL\01\6c\02\00\7c\01\7e\02\7e\01\00\2a\01\00"                   !: (record {}) "record: type too long";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 25");
      // TODO: roundtrip decoding check
  } {
      // assert blob "DIDL\01\6c\02\00\01\01\7e\01\00\00\00"                            !: (record {}) "record: type out of range";
      // didc encode ...
      // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 26");
      // TODO: roundtrip decoding check
  } {
    // assert blob "DIDL\01\6c\02\00\01\01\7e\01\7c\2a"                               !: (reserved) "record: type out of range";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 27");
    // TODO: roundtrip decoding check
  }
}

int compliance_test_construct() {
  IC_API::debug_print(" - " + std::string(__func__));
  test_records();
  return 0;
}
