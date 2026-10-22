// https://github.com/dfinity/candid/blob/master/test/construct.test.did

#include "compliance_test_construct.h"
#include "candid_type.h"

#include <optional>
#include <string>

#include "candid_deserialize.h"
#include "candid_serialize.h"
#include "candid_serialize_type_table_registry.h"
#include "ic_api.h"

void test_records() {
  IC_API::debug_print("   - " + std::string(__func__));
  // record
  {
    CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\00\01\00" == "(record {})"                                : (record {}) "record: empty";
    // didc encode '(record {})'
    CandidTypeRecord r;
    if (CandidSerialize(r).assert_candid("4449444c016c000100", true))
      IC_API::trap(std::string(__func__) + ": 1");
    // TODO: roundtrip decoding check
  }
  {
    CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\00\03\00\00\00" == "(record {}, record {}, record {})"    : (record {}, record {}, record {}) "record: multiple records";
    // didc encode '(record {}, record {}, record {})'
    // These alternative encodings are OK:
    // 4449444c036c006c006c0003000102 : using 3 entries in the type table, and reference the type table index
    // 4449444c016c0003000000         : compressed format, with 1 entry in the type table, referenced 3 times, because values are the same

    CandidTypeRecord r1;
    CandidTypeRecord r2;
    CandidTypeRecord r3;
    CandidArgs A;
    A.append(r1);
    A.append(r2);
    A.append(r3);
    if (CandidSerialize(A).assert_candid("4449444c016c0003000000", true))
      IC_API::trap(std::string(__func__) + ": 2");
    // TODO: roundtrip decoding check
    // We do not (yet) check if the byte stream can be compressed.
    // It seems counter effective actually to check on compression by default.
    // if (CandidSerialize(A).assert_candid("4449444c016c0003000000", true))
    //   IC_API::trap("2");
  }
  {
    CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\01\01\7c\01\00\2a" == "(record { 1 = 42 })"               : (record {1:int}) "record";
    // didc encode '(record { 1 = 42 : int })'
    CandidTypeRecord r;
    r.append(1, CandidTypeInt(42));
    if (CandidSerialize(r).assert_candid("4449444c016c01017c01002a", true))
      IC_API::trap(std::string(__func__) + ": 3");
    // TODO: roundtrip decoding check
  }
  {
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\01\01\7c\01\00\2a" == "(record { 1 = opt 42 })"           : (record {1:opt int}) "record: opt";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 4");
    // TODO: roundtrip decoding check
  }
  {
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\01\01\7c\01\00\2a" == "(record { 1 = null })"             : (record {1:reserved}) "record: reserved";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 5");
    // TODO: roundtrip decoding check
  }
  {
    CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\01\01\7c\01\00\2a" == "(record {})"                       : (record {}) "record: ignore fields";
    // decoding check: an additional field of any type is skipped
    CandidArgs A;
    A.append(CandidTypeRecord());
    CandidDeserialize("4449444c016c01017c01002a", A);
  }
  {
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert "(record { whatever = 0 })"  == "(record {})"                              : (record {}) "record: ignore fields (textual)";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 7");
    // TODO: roundtrip decoding check
  }
  {
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\01\01\7c\01\00\2a"                                       !: (record {2:int}) "record: missing field";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 8");
    // TODO: roundtrip decoding check
  }
  {
    CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\01\01\7c\01\00\2a" == "(record { 2 = null })"             : (record {2:opt int}) "record: missing opt field";
    // decoding check
    std::optional<__int128_t> o;
    CandidTypeRecord r;
    r.append(2, CandidTypeOptInt(&o));
    CandidArgs A;
    A.append(r);
    CandidDeserialize("4449444c016c01017c01002a", A);
    if (o.has_value()) IC_API::trap(std::string(__func__) + ": 9");
  }
  {
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\02\00\7c\01\7e\01\00\2a\01" == "(record {42; true})"      : (record {int; bool}) "record: tuple";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 10");
    // TODO: roundtrip decoding check
  }
  {
    CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\02\00\7c\01\7e\01\00\2a\01" == "(record {1 = true})"      : (record {1:bool}) "record: ignore fields";
    // decoding check
    bool b{false};
    CandidTypeRecord r;
    r.append(1, CandidTypeBool(&b));
    CandidArgs A;
    A.append(r);
    CandidDeserialize("4449444c016c02007c017e01002a01", A);
    if (!b) IC_API::trap(std::string(__func__) + ": 11");
  }
  {
    CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\07\6c\07..." == "(record { bar = true; baz = record { bar = false }; bbb = opt record { head = 20 }})"
    //   : (record { bar: bool; baz: record { bar: bool }; bbb: opt record { head : int} }) "skip fields";
    // decoding check: additional fields are skipped, also in nested records
    bool bar{false};
    bool baz_bar{true};
    __int128_t head{0};
    bool has_bbb{false};
    CandidTypeRecord baz;
    baz.append("bar", CandidTypeBool(&baz_bar));
    CandidTypeRecord bbb;
    bbb.append("head", CandidTypeInt(&head));
    CandidTypeRecord r;
    r.append("bar", CandidTypeBool(&bar));
    r.append("baz", baz);
    r.append("bbb", CandidTypeOptRecord(&bbb, &has_bbb));
    CandidArgs A;
    A.append(r);
    CandidDeserialize(
        "4449444c076c07c3e3aa0201d3e3aa027ed5e3aa0202dbe3aa0201a2e5aa0204bbf1aa0206868eb702756c02d3e3aa027e868eb702756b02d1a7cf027ff1f3928e04036c02a0d2aca8047c90eddae704026e056c02a0d2aca8047c90eddae704046b02d3e3aa027f868eb7027f0100010b0000000100000a000000011400002a000000",
        A);
    if (!bar || baz_bar || !has_bbb || head != 20)
      IC_API::trap(std::string(__func__) + ": skip fields");
  }
  {
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\02\00\7c\01\7e\01\00\2a\01"                              !: (record {bool; int}) "record: type mismatch";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 12");
    // TODO: roundtrip decoding check
  }
  {
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\02\00\7c\00\7e\01\00\2a\01"                              !: (record {int; bool}) "record: duplicate fields";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 13");
    // TODO: roundtrip decoding check
  }
  {
    CandidSerializeTypeTableRegistry::get_instance().clear();
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
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\02\d3\e3\aa\02\7e\86\8e\b7\02\7c\01\00\01\2a"
    //    == "(record { foo = 42; bar = true})"                                         : (record {foo:int; bar:bool}) "record: named fields";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 15");
    // TODO: roundtrip decoding check
  }
  {
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\01\cd\84\b0\05\7f\01\00" == "(record { \"☃\" = null })"   : (record { "☃":null }) "record: unicode field";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 16");
    // TODO: roundtrip decoding check
  }
  {
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\01\80\e4\97\d0\12\7c\01\00\2a"                         !: (record {}) "record: field hash larger than u32";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 17");
    // TODO: roundtrip decoding check
  }
  {
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\04\6c\01\00\01\6c\01\00\02\6c\01\00\03\6c\00\01\00"
    //    == "(record { record { record { record {} } } })"                           : (record{record{record{record{}}}}) "record: nested";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 18");
    // TODO: roundtrip decoding check
  }
  {
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\01\00\00\01\00"                                        !: (EmptyRecord) "record: empty recursion";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 19");
    // TODO: roundtrip decoding check
  }
  {
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\02\6d\01\6c\01\00\01\01\00\01"                               !: (vec EmptyRecord) "vec of empty records";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 20");
    // TODO: roundtrip decoding check
  }
  {
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\02\00\7c\01\7e\01\00\2a"                               !: (record {int}) "record: value too short";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 21");
    // TODO: roundtrip decoding check
  }
  {
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\02\00\7c\01\7e\01\00\2a\01\00"                         !: (record {int}) "record: value too long";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 22");
    // TODO: roundtrip decoding check
  }
  {
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\02\00\7c\01\01\00\2a\01"                               !: (record {}) "record: type too short";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 23");
    // TODO: roundtrip decoding check
  }
  {
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\02\00\7c\01\00\2a\01"                                  !: (record {}) "record: type too short";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 24");
    // TODO: roundtrip decoding check
  }
  {
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    //assert blob "DIDL\01\6c\02\00\7c\01\7e\02\7e\01\00\2a\01\00"                   !: (record {}) "record: type too long";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 25");
    // TODO: roundtrip decoding check
  }
  {
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\02\00\01\01\7e\01\00\00\00"                            !: (record {}) "record: type out of range";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 26");
    // TODO: roundtrip decoding check
  }
  {
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // assert blob "DIDL\01\6c\02\00\01\01\7e\01\7c\2a"                               !: (reserved) "record: type out of range";
    // didc encode ...
    // TODO if (CandidSerialize(r).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 27");
    // TODO: roundtrip decoding check
  }
}

void test_vectors() {
  IC_API::debug_print("   - " + std::string(__func__));
  {
    // assert blob "DIDL\02\6d\01\6c\00\01\00\05"                                  : (vec record {}) "vec of records";
    // decoding check, with CandidTypeVecOf
    CandidSerializeTypeTableRegistry::get_instance().clear();
    size_t count{0};
    CandidArgs A;
    A.append(CandidTypeVecOf([&]() -> CandidType { return CandidTypeRecord(); },
                             [&]() { ++count; }));
    CandidDeserialize("4449444c026d016c00010005", A);
    if (count != 5) IC_API::trap(std::string(__func__) + ": vec of records");
  }
  // Not yet supported: a vec null is refused when the type table is read
  // assert blob "DIDL\01\6d\7f\01\00\e8\07"                                     : (vec null) "vec: space bomb";
}

int compliance_test_construct() {
  IC_API::debug_print(" - " + std::string(__func__));
  test_records();
  test_vectors();
  // Clear out the type table registry
  CandidSerializeTypeTableRegistry::get_instance().clear();
  return 0;
}
