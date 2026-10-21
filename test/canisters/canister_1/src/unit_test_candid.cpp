// Unit tests for ic/candid/*.cpp

#include "unit_test_candid.h"
#include "candid_type.h"

#include "candid_type_all_includes.h"

#include "candid_type_visitors.h"

#include <algorithm>
#include <optional>
#include <string>

#include "candid_deserialize.h"
#include "candid_serialize.h"
#include "candid_serialize_type_table_registry.h"
#include "ic_api.h"

namespace {
// The hex of a record with the fields 10 (nat) & 20 (text), and additional
// fields of many types before, between and after them. wrap puts the record
// in a variant, an opt or a vec.
enum class Wrap { None, Variant, Opt };
std::string superset_hex(__uint128_t n, const std::string &t,
                         Wrap wrap = Wrap::None) {
  // The type table registry must be cleared before the types are built
  CandidSerializeTypeTableRegistry::get_instance().clear();
  CandidTypeRecord inner;
  inner.append(1, CandidTypeText("inner"));
  inner.append(2, CandidTypeVecNat8(std::vector<uint8_t>{1, 2, 3}));

  CandidTypeRecord r;
  r.append(3, CandidTypeOptRecord(inner));
  r.append(5, CandidTypeNat(__uint128_t(5)));
  r.append(10, CandidTypeNat(n));
  r.append(15, CandidTypeText("between"));
  r.append(16, inner);
  r.append(17, CandidTypeVariant("x", CandidTypeInt(__int128_t(-7))));
  r.append(20, CandidTypeText(t));
  r.append(25, CandidTypeVecNat(std::vector<__uint128_t>{1, 2}));
  r.append(30, CandidTypeVecNat8(std::vector<uint8_t>{0xde, 0xad}));
  r.append(35, CandidTypePrincipal("aaaaa-aa"));
  r.append(40, CandidTypeRecord());
  r.append(45, CandidTypeBool(true));
  if (wrap == Wrap::Variant)
    return CandidSerialize(CandidTypeVariant("ok", r)).as_hex_string();
  if (wrap == Wrap::Opt)
    return CandidSerialize(CandidTypeOptRecord(r)).as_hex_string();
  return CandidSerialize(r).as_hex_string();
}

void decode(const std::string &hex_string, const CandidType &expected) {
  CandidArgs A;
  A.append(expected);
  CandidDeserialize(hex_string, A);
}

void check(bool ok, const std::string &what) {
  if (!ok) IC_API::trap("test_record_width_subtyping: " + what);
}

void test_record_width_subtyping() {
  { // A record
    std::string wire = superset_hex(42, "hello");
    __uint128_t n{0};
    std::string t;
    CandidTypeRecord r;
    r.append(10, CandidTypeNat(&n));
    r.append(20, CandidTypeText(&t));
    decode(wire, r);
    check(n == 42 && t == "hello", "record");
  }

  { // A record in a variant case
    std::string wire = superset_hex(43, "in variant", Wrap::Variant);
    __uint128_t n{0};
    std::string t;
    std::string label;
    CandidTypeRecord r;
    r.append(10, CandidTypeNat(&n));
    r.append(20, CandidTypeText(&t));
    CandidTypeVariant v{&label};
    v.append("ok", r);
    decode(wire, v);
    check(label == "ok" && n == 43 && t == "in variant", "variant");
  }

  { // A record in an opt
    std::string wire = superset_hex(44, "in opt", Wrap::Opt);
    __uint128_t n{0};
    std::string t;
    bool has_value{false};
    CandidTypeRecord r;
    r.append(10, CandidTypeNat(&n));
    r.append(20, CandidTypeText(&t));
    decode(wire, CandidTypeOptRecord(&r, &has_value));
    check(has_value && n == 44 && t == "in opt", "opt");
  }

  { // Records in a vec
    CandidSerializeTypeTableRegistry::get_instance().clear();
    CandidTypeRecord w;
    w.append(3, CandidTypeVecNat(std::vector<__uint128_t>{7, 8}));
    w.append(10, CandidTypeVecNat(std::vector<__uint128_t>{45, 46}));
    w.append(15, CandidTypeVecText(std::vector<std::string>{"x", "y"}));
    w.append(20, CandidTypeVecText(std::vector<std::string>{"a", "b"}));
    w.append(25, CandidTypeVecBool(std::vector<bool>{true, false}));
    std::string wire = CandidSerialize(CandidTypeVecRecord(w)).as_hex_string();

    std::vector<__uint128_t> ns;
    std::vector<std::string> ts;
    CandidTypeRecord r;
    r.append(10, CandidTypeVecNat(&ns));
    r.append(20, CandidTypeVecText(&ts));
    decode(wire, CandidTypeVecRecord(&r));
    check(ns == std::vector<__uint128_t>{45, 46} &&
              ts == std::vector<std::string>{"a", "b"},
          "vec");
  }

  { // An expected opt that is not on the wire, after an additional field
    std::string wire = superset_hex(47, "no opt");
    __uint128_t n{0};
    std::string t;
    std::optional<std::string> o;
    CandidTypeRecord r;
    r.append(10, CandidTypeNat(&n));
    r.append(20, CandidTypeText(&t));
    r.append(99, CandidTypeOptText(&o));
    decode(wire, r);
    check(n == 47 && t == "no opt" && !o.has_value(), "missing opt");
  }

  { // A record with only additional fields
    std::string wire = superset_hex(48, "all extra");
    decode(wire, CandidTypeRecord());
  }

  // The tests that follow expect an empty type table registry
  CandidSerializeTypeTableRegistry::get_instance().clear();
}
} // namespace

int unit_test_candid() {
  { // Verify idl hash
    CandidSerializeTypeTableRegistry::get_instance().clear();
    if (CandidTypeBase<CandidTypeRecord>().idl_hash("syndactyle") != 4260381820)
      IC_API::trap(std::string(__func__) + ": 1a");
    if (CandidTypeBase<CandidTypeRecord>().idl_hash("rectum") != 4260381820)
      IC_API::trap(std::string(__func__) + ": 1b");
    if (CandidTypeBase<CandidTypeRecord>().idl_hash("trustbuster") !=
        3504418361)
      IC_API::trap(std::string(__func__) + ": 1c");
    if (CandidTypeBase<CandidTypeRecord>().idl_hash("destroys") != 3504418361)
      IC_API::trap(std::string(__func__) + ": 1d");
  }
  {
    // Verify that the CandidType '<' operator enables sorting
    // This does not compile to wasm
    // CandidSerializeTypeTableRegistry::get_instance().clear();
    // std::vector<CandidType> vec;
    // vec.push_back(CandidTypeBool(true));
    // vec.push_back(CandidTypeInt(42));
    // vec.push_back(CandidTypeText("hello!"));
    // std::sort(vec.begin(), vec.end());
    // CandidTypeRoot *ptr_prev = get_candid_type_root_ptr(vec[0]);
    // int opcode_prev = ptr_prev->get_datatype_opcode();
    // for (CandidType v : vec) {
    //   CandidTypeRoot *ptr_v = get_candid_type_root_ptr(v);
    //   int opcode = ptr_v->get_datatype_opcode();
    //   if (opcode_prev > opcode) IC_API::trap(std::string(__func__) + ": 2");
    //   opcode_prev = opcode;
    // }
  }

  { // Verify the VecBytes static utils
    CandidSerializeTypeTableRegistry::get_instance().clear();
    const std::byte b = std::byte(42);
    if (VecBytes::byte_to_hex(b) != "0x2a")
      IC_API::trap(std::string(__func__) + ": 3a");

    if (VecBytes::byte_to_hex(b, "") != "2a")
      IC_API::trap(std::string(__func__) + ": 3b");

    if (VecBytes::byte_to_dec(b) != "      42")
      IC_API::trap(std::string(__func__) + ": 3c");

    if (VecBytes::byte_to_dec(b, 2) != "42")
      IC_API::trap(std::string(__func__) + ": 3d");

    if (VecBytes::byte_to_char(b) != "*")
      IC_API::trap(std::string(__func__) + ": 3e");

    if (IC_API::to_string_128(__uint128_t(101)) != "101")
      IC_API::trap(std::string(__func__) + ": 3f");

    if (IC_API::to_string_128(__int128_t(101)) != "101")
      IC_API::trap(std::string(__func__) + ": 3f");

    if (IC_API::to_string_128(__int128_t(-101)) != "-101")
      IC_API::trap(std::string(__func__) + ": 3f");

    {
      std::string max_uint128 = "340282366920938463463374607431768211455";
      __uint128_t i = VecBytes::atouint128_t(max_uint128);
      if (IC_API::to_string_128(i) != max_uint128)
        IC_API::trap(std::string(__func__) + ": 3g");
    }
  }
  { // Verify serialization & deserializtion of 2 records
    // didc encode '(record { 6 = 42 : int; 9 = 43 : int }, record { 7 = 44 : int; 10 = 45 : int })'

    { // serialize

      CandidSerializeTypeTableRegistry::get_instance().clear();
      CandidTypeRecord r1;
      r1.append(6, CandidTypeInt(42));
      r1.append(9, CandidTypeInt(43));
      //
      CandidTypeRecord r2;
      r2.append(7, CandidTypeInt(44));
      r2.append(10, CandidTypeInt(45));
      //
      CandidArgs A;
      A.append(r1);
      A.append(r2);
      //
      if (CandidSerialize(A).assert_candid(
              "4449444c026c02067c097c6c02077c0a7c0200012a2b2c2d", true))
        IC_API::trap(std::string(__func__) + ": 4a");
    }

    { // deserialize
      CandidSerializeTypeTableRegistry::get_instance().clear();
      __int128_t r1_i1;
      __int128_t r1_i2;
      CandidTypeRecord r1;
      r1.append(6, CandidTypeInt(&r1_i1));
      r1.append(9, CandidTypeInt(&r1_i2));
      //
      __int128_t r2_i1;
      __int128_t r2_i2;
      CandidTypeRecord r2;
      r2.append(7, CandidTypeInt(&r2_i1));
      r2.append(10, CandidTypeInt(&r2_i2));
      //
      CandidArgs A;
      A.append(r1);
      A.append(r2);
      //
      CandidDeserialize("4449444c026c02067c097c6c02077c0a7c0200012a2b2c2d", A);
      if (r1_i1 != 42 || r1_i2 != 43 || r2_i1 != 44 || r2_i2 != 45)
        IC_API::trap(std::string(__func__) + ": 4b");
      // std::string name = record_in["name"];
      // double secret_x = record_in["secret float64"];
      // int secret_i = record_in["secret int"];}
    }
  }

  { // Verify serialization variant of tiny Stories example
    // didc encode '(variant { ok = "Once upon a time, there was a little girl named Lily. She loved to play outside in the sunshine. One day, she saw a big, red ball in the sky. It was the sun! She thought it was so pretty.\nLily wanted to play with the ball, but it was too high up in the sky. She tried to jump and reach it, but she couldn't. Then, she had an idea. She would use a stick to knock the" : text })'

    CandidSerializeTypeTableRegistry::get_instance().clear();
    CandidArgs A;
    std::string output =
        "Once upon a time, there was a little girl named Lily. She loved to play outside in the sunshine. One day, she saw a big, red ball in the sky. It was the sun! She thought it was so pretty.\nLily wanted to play with the ball, but it was too high up in the sky. She tried to jump and reach it, but she couldn\'t. Then, she had an idea. She would use a stick to knock the";
    A.append(CandidTypeVariant{"ok", CandidTypeText{output}});

    std::string s = CandidSerialize(A).as_hex_string();
    if (s !=
        "4449444c016b019cc20171010000ed024f6e63652075706f6e20612074696d652c207468657265207761732061206c6974746c65206769726c206e616d6564204c696c792e20536865206c6f76656420746f20706c6179206f75747369646520696e207468652073756e7368696e652e204f6e65206461792c20736865207361772061206269672c207265642062616c6c20696e2074686520736b792e20497420776173207468652073756e21205368652074686f756768742069742077617320736f207072657474792e0a4c696c792077616e74656420746f20706c61792077697468207468652062616c6c2c206275742069742077617320746f6f206869676820757020696e2074686520736b792e2053686520747269656420746f206a756d7020616e642072656163682069742c206275742073686520636f756c646e27742e205468656e2c207368652068616420616e20696465612e2053686520776f756c6420757365206120737469636b20746f206b6e6f636b20746865")
      IC_API::trap(std::string(__func__) + ": 4c");
  }

  { // Verify serialization & deserialization of CandidTypeVecVariant (unit variants)
    // didc encode '(vec { variant { AdminQuery }; variant { AdminUpdate }; variant { AdminQuery } })'
    // Note: We test unit variants where the variant options have no associated data

    { // serialize
      CandidSerializeTypeTableRegistry::get_instance().clear();

      // Create a template variant with the valid options
      CandidTypeVariant v_template;
      v_template.append("AdminQuery", CandidTypeNull{});
      v_template.append("AdminUpdate", CandidTypeNull{});

      // Vector of selected labels
      std::vector<std::string> labels = {"AdminQuery", "AdminUpdate",
                                         "AdminQuery"};

      CandidArgs A;
      A.append(CandidTypeVecVariant{v_template, labels});

      std::string s = CandidSerialize(A).as_hex_string();
      // Verify it starts with DIDL and has expected structure
      if (s.substr(0, 8) != "4449444c")
        IC_API::trap(std::string(__func__) + ": vec_variant 1a - bad magic");
      // The hex should encode: vec of 3 variants with indices 0, 1, 0
      // Type table + vec length (3) + values (0, 1, 0)
    }

    { // deserialize
      CandidSerializeTypeTableRegistry::get_instance().clear();

      // Create a template variant with the valid options
      CandidTypeVariant v_template;
      v_template.append("AdminQuery", CandidTypeNull{});
      v_template.append("AdminUpdate", CandidTypeNull{});

      // Vector to receive the decoded labels
      std::vector<std::string> labels;

      CandidArgs A;
      A.append(CandidTypeVecVariant{&v_template, &labels});

      // First serialize to get the hex string
      CandidSerializeTypeTableRegistry::get_instance().clear();
      CandidTypeVariant v_template_encode;
      v_template_encode.append("AdminQuery", CandidTypeNull{});
      v_template_encode.append("AdminUpdate", CandidTypeNull{});
      std::vector<std::string> labels_encode = {"AdminQuery", "AdminUpdate",
                                                "AdminQuery"};
      CandidArgs A_encode;
      A_encode.append(CandidTypeVecVariant{v_template_encode, labels_encode});
      std::string hex = CandidSerialize(A_encode).as_hex_string();

      // Now deserialize
      CandidSerializeTypeTableRegistry::get_instance().clear();
      CandidDeserialize(hex, A);

      if (labels.size() != 3)
        IC_API::trap(std::string(__func__) + ": vec_variant 1b - wrong size");
      if (labels[0] != "AdminQuery")
        IC_API::trap(std::string(__func__) + ": vec_variant 1c - labels[0]");
      if (labels[1] != "AdminUpdate")
        IC_API::trap(std::string(__func__) + ": vec_variant 1d - labels[1]");
      if (labels[2] != "AdminQuery")
        IC_API::trap(std::string(__func__) + ": vec_variant 1e - labels[2]");
    }
  }

  { // A variant's fields go on the wire sorted by id (hash), whatever the
    // append order. Hash order is noul, score, choice: append in another order.
    // Verified with an independent decoder (icp-py-core's icp_candid.decode):
    // '(vec { variant { choice }; variant { noul }; variant { score } })'
    CandidSerializeTypeTableRegistry::get_instance().clear();

    CandidTypeVariant v_template;
    v_template.append("choice", CandidTypeNull{});
    v_template.append("score", CandidTypeNull{});
    v_template.append("noul", CandidTypeNull{});

    std::vector<std::string> labels = {"choice", "noul", "score"};

    CandidArgs A;
    A.append(CandidTypeVecVariant{v_template, labels});

    std::string s = CandidSerialize(A).as_hex_string();
    if (s !=
        "4449444c046b03d8b1a8c8047fd2e6e5c6077fe1febe850c7f6b006b03d8b1a8c8047fd2e6e5c6077fe1febe850c7f6d02010303020001")
      IC_API::trap(std::string(__func__) + ": vec_variant 2");
  }

  { // An implicit variant field id is the previously appended id + 1, even
    // though the fields are kept sorted by id
    CandidSerializeTypeTableRegistry::get_instance().clear();

    CandidTypeVariant v;
    v.append(CandidTypeNull{});
    if (v.get_field_ids() != std::vector<uint32_t>{0})
      IC_API::trap(std::string(__func__) + ": variant implicit id 1");

    v.append(uint32_t(100), CandidTypeNull{});
    v.append(uint32_t(5), CandidTypeNull{});
    v.append(CandidTypeNull{});
    if (v.get_field_ids() != std::vector<uint32_t>{0, 5, 6, 100})
      IC_API::trap(std::string(__func__) + ": variant implicit id 2");
  }

  // Verify CandidTypePrincipal (https://internetcomputer.org/docs/current/references/id-encoding-spec#decode)
  {
    CandidSerializeTypeTableRegistry::get_instance().clear();
    std::string s = "2ibo7-dia";
    CandidTypePrincipal p = CandidTypePrincipal(s);
    std::vector<std::byte> v = p.get_M().vec();
    std::vector<std::byte> v_expected{std::byte('\x01'), std::byte('\x01'),
                                      std::byte('\x00')};
    if (v != v_expected) IC_API::trap(std::string(__func__) + ": 5a");
  }
  {
    CandidSerializeTypeTableRegistry::get_instance().clear();
    std::string s = "w3gef-eqbai";
    CandidTypePrincipal p = CandidTypePrincipal(s);
    std::vector<std::byte> v = p.get_M().vec();
    std::vector<std::byte> v_expected{std::byte('\x01'), std::byte('\x02'),
                                      std::byte('\x01'), std::byte('\x02')};
    if (v != v_expected) IC_API::trap(std::string(__func__) + ": 5b");
  }

  { // Record width subtyping: an additional field on the wire, of any type,
    // is decoded and discarded, also inside a variant, an opt and a vec
    test_record_width_subtyping();
  }

  return 0;
}