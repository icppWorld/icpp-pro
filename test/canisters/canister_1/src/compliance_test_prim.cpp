// https://github.com/dfinity/candid/blob/master/test/construct.test.did

#include "compliance_test_prim.h"

#include <cmath>
#include <string>

#include "ic_api.h"

void test_wrong() {
  IC_API::debug_print("   - " + std::string(__func__));
  // fundamentally wrong

  {
      // assert blob ""              !: () "empty";
      // decoding check
      // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 1");
  } {
      // assert blob "\00\00"        !: () "no magic bytes";
      // decoding check
      // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 2");
  } {
      // assert blob "DADL"          !: () "wrong magic bytes";
      // decoding check
      // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 3");
  } {
      // assert blob "DADL\00\00"    !: () "wrong magic bytes";
      // decoding check
      // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 4");
  } {
      // assert blob "DIDL\80\00\00"  : () "overlong typ table length";
      // decoding check
      // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 5");
  } {
    // assert blob "DIDL\00\80\00"  : () "overlong arg length";
    // decoding check
    // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 6");
  }
}

void test_nullary_input() {
  IC_API::debug_print("   - " + std::string(__func__));
  // nullary input
  {
    // assert blob "DIDL\00\00"     : ();
    // didc encode '()'
    if (CandidSerialize().assert_candid("4449444c0000", true))
      IC_API::trap(std::string(__func__) + ": 1");
    // TODO: roundtrip decoding check
  }
  {
      // assert blob "DIDL\00\00\00" !: () "nullary: too long";
      // decoding check
      // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 2");
  } {
      // assert blob "DIDL\00\01\7f"  : () "Additional parameters are ignored";
      // decoding check
      // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 3");
  } {
      // assert blob "DIDL\00\01\6e" !: () "Not a primitive type";
      // decoding check
      // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 4");
  } {
    // assert blob "DIDL\00\01\5e" !: () "Out of range type";
    // decoding check
    // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 5");
  }
}

void test_missing_arguments() {
  IC_API::debug_print("   - " + std::string(__func__));
  // Missing arguments

  {
      // assert blob "DIDL\00\00"                        !: (nat)       "missing argument: nat   fails";
      // decoding check
      // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 1");
  } {
      // assert blob "DIDL\00\00"                        !: (empty)     "missing argument: empty fails";
      // decoding check
      // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 2");
  } {
      // assert blob "DIDL\00\00"                        !: (null)      "missing argument: null  fails";
      // decoding check
      // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 3");
  } {
      // assert blob "DIDL\00\00" == "(null)"             : (opt empty) "missing argument: opt empty";
      // decoding check
      // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 4");
  } {
      // assert blob "DIDL\00\00" == "(null)"             : (opt null)  "missing argument: opt null";
      // decoding check
      // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 5");
  } {
      // assert blob "DIDL\00\00" == "(null)"             : (opt nat)   "missing argument: opt nat";
      // decoding check
      // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 6");
  } {
    // assert blob "DIDL\00\00" == blob "DIDL\00\01\70" : (reserved)  "missing argument: reserved";
    // decoding check
    // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 7");
  }
}

void test_null() {
  IC_API::debug_print("   - " + std::string(__func__));
  // primitive types: null
  {
    // assert blob "DIDL\00\01\7f"     : (null);
    // didc encode '(null : null)'
    CandidTypeNull c;
    if (CandidSerialize(c).assert_candid("4449444c00017f", true))
      IC_API::trap(std::string(__func__) + ": 1");
    // TODO: roundtrip decoding check
  }
  {
      // assert blob "DIDL\00\01\7e"    !: (null) "wrong type";
      // decoding check
      // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 2");
  } {
    // assert blob "DIDL\00\01\7f\00" !: (null) "null: too long";
    // decoding check
    // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 3");
  }
}

void test_bool() {
  IC_API::debug_print("   - " + std::string(__func__));
  // primitive types: bool
  {
    // assert blob "DIDL\00\01\7e\00" == "(false)" : (bool) "bool: false";
    // didc encode '(false)'
    CandidTypeBool c(false);
    if (CandidSerialize(c).assert_candid("4449444c00017e00", true))
      IC_API::trap(std::string(__func__) + ": 1");
    // TODO: roundtrip decoding check
  }
  {
    // assert blob "DIDL\00\01\7e\01" == "(true)" : (bool) "bool: true";
    // didc encode '(true)'
    CandidTypeBool c(true);
    if (CandidSerialize(c).assert_candid("4449444c00017e01", true))
      IC_API::trap(std::string(__func__) + ": 2");
    // TODO: roundtrip decoding check
  }
  {
      // assert blob "DIDL\00\01\7e"                !: (bool) "bool: missing";
      // decoding check
      // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 3");
  } {
      // assert blob "DIDL\00\01\7e\02"             !: (bool) "bool: out of range";
      // decoding check
      // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 4");
  } {
    // assert blob "DIDL\00\01\7e\ff"             !: (bool) "bool: out of range";
    // decoding check
    // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 5");
  }
}

void test_nat() {
  IC_API::debug_print("   - " + std::string(__func__));
  // primitive types: nat
  {
    // assert blob "DIDL\00\01\7d\00" == "(0)"         : (nat) "nat: 0";
    // didc encode '(0: nat)'
    CandidTypeNat c(0);
    if (CandidSerialize(c).assert_candid("4449444c00017d00", true))
      IC_API::trap(std::string(__func__) + ": 1");
    // TODO: roundtrip decoding check
  }
  {
    // assert blob "DIDL\00\01\7d\01" == "(1)"         : (nat) "nat: 1";
    // didc encode '(1: nat)'
    CandidTypeNat c(1);
    if (CandidSerialize(c).assert_candid("4449444c00017d01", true))
      IC_API::trap(std::string(__func__) + ": 2");
    // TODO: roundtrip decoding check
  }
  {
    // assert blob "DIDL\00\01\7d\7f" == "(127)"       : (nat) "nat: 0x7f";
    // didc encode '(127: nat)'
    CandidTypeNat c(127);
    if (CandidSerialize(c).assert_candid("4449444c00017d7f", true))
      IC_API::trap(std::string(__func__) + ": 3");
    // TODO: roundtrip decoding check
  }
  {
    // assert blob "DIDL\00\01\7d\80\01"  == "(128)"   : (nat) "nat: leb (two bytes)";
    // didc encode '(128: nat)'
    CandidTypeNat c(128);
    if (CandidSerialize(c).assert_candid("4449444c00017d8001", true))
      IC_API::trap(std::string(__func__) + ": 4");
    // TODO: roundtrip decoding check
  }
  {
    // assert blob "DIDL\00\01\7d\ff\7f"  == "(16383)" : (nat) "nat: leb (two bytes, all bits)";
    // didc encode '(16383: nat)'
    CandidTypeNat c(16383);
    if (CandidSerialize(c).assert_candid("4449444c00017dff7f", true))
      IC_API::trap(std::string(__func__) + ": 5");
    // TODO: roundtrip decoding check
  }
  {
      // assert blob "DIDL\00\01\7d\80"                 !: (nat) "nat: leb too short";
      // decoding check
      // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 6");
  } {
      // assert blob "DIDL\00\01\7d\80\00" == "(0)"      : (nat) "nat: leb overlong";
      // decoding check
      // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 7");
  } {
      // assert blob "DIDL\00\01\7d\ff\00" == "(127)"    : (nat) "nat: leb overlong";
      // decoding check
      // TODO if (CandidSerialize(c).assert_candid("...", true)) IC_API::trap(std::string(__func__) + ": 8");
  } {
    // assert blob "DIDL\00\01\7d\80\80\98\f4\e9\b5\ca\6a" == "(60000000000000000)" : (nat) "nat: big number";
    // didc encode '(60000000000000000: nat)'
    CandidTypeNat c(60000000000000000);
    if (CandidSerialize(c).assert_candid("4449444c00017d808098f4e9b5ca6a",
                                         true))
      IC_API::trap(std::string(__func__) + ": 9a");
    // TODO: roundtrip decoding check
  }
  {
    // We added another big number test. The biggest possible value in 128 bits
    // didc encode '(340282366920938463463374607431768211455: nat)'
    __uint128_t i =
        VecBytes::atouint128_t("340282366920938463463374607431768211455");
    CandidTypeNat c(i);
    if (CandidSerialize(c).assert_candid(
            "4449444c00017dffffffffffffffffffffffffffffffffffff03", true))
      IC_API::trap(std::string(__func__) + ": 9b");
    // TODO: roundtrip decoding check
  }
  {
    // // TODO: This is giving an overflow because we use __uint128_t
    // // To be compliant with Candid spec, we need to use a big int library or something like that...
    // // didc encode '(340282366920938463463374607431768211456: nat)'
    // __uint128_t i =
    //     VecBytes::atouint128_t("340282366920938463463374607431768211456");
    // CandidTypeNat c(i);
    // if (CandidSerialize(c).assert_candid(
    //         "4449444c00017d80808080808080808080808080808080808004", true))
    //   IC_API::trap(std::string(__func__) + ": 9b");
    // // TODO: roundtrip decoding check
  }
}

int compliance_test_prim() {
  IC_API::debug_print(" - " + std::string(__func__));
  test_wrong();
  test_nullary_input();
  test_missing_arguments();
  test_null();
  test_bool();
  test_nat();
  return 0;
}
