// Unit tests for ic/candid/*.cpp

#include "unit_test_candid.h"
#include "candid_type.h"

#include "candid_type_all_includes.h"

#include "candid_type_visitors.h"

#include <algorithm>
#include <string>

#include "candid_deserialize.h"
#include "candid_serialize.h"
#include "candid_serialize_type_table_registry.h"
#include "ic_api.h"

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
  {// Verify serialization & deserializtion of 2 records
   // didc encode '(record { 6 = 42 : int; 9 = 43 : int }, record { 7 = 44 : int; 10 = 45 : int })'

   {// serialize

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

return 0;
}