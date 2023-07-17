// Unit tests for ic/candid/*.cpp

#include "unit_test_candid.h"

#include <algorithm>
#include <string>

#include "candid_deserialize.h"
#include "candid_serialize.h"
#include "ic_api.h"

int unit_test_candid() {
  { // Verify idl hash
    if (CandidTypeBase().idl_hash("syndactyle") != 4260381820)
      IC_API::trap(std::string(__func__) + ": 1a");
    if (CandidTypeBase().idl_hash("rectum") != 4260381820)
      IC_API::trap(std::string(__func__) + ": 1b");
    if (CandidTypeBase().idl_hash("trustbuster") != 3504418361)
      IC_API::trap(std::string(__func__) + ": 1c");
    if (CandidTypeBase().idl_hash("destroys") != 3504418361)
      IC_API::trap(std::string(__func__) + ": 1d");
  }
  { // Verify that the CandidTypeBase '<' operator enables sorting
    std::vector<CandidTypeBase> vec;
    vec.push_back(CandidTypeBool(true));
    vec.push_back(CandidTypeInt(42));
    vec.push_back(CandidTypeText("hello!"));
    std::sort(vec.begin(), vec.end());
    int opcode_prev = vec[0].get_datatype_opcode();
    for (CandidTypeBase v : vec) {
      int opcode = v.get_datatype_opcode();
      if (opcode_prev > opcode) IC_API::trap(std::string(__func__) + ": 2");
      opcode_prev = opcode;
    }
  }

  { // Verify the VecBytes static utils
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

// Verify CandidTypePrincipal (https://internetcomputer.org/docs/current/references/id-encoding-spec#decode)
{
  std::string s = "2ibo7-dia";
  CandidTypePrincipal p = CandidTypePrincipal(s);
  std::vector<std::byte> v = p.get_M().vec();
  std::vector<std::byte> v_expected{std::byte('\x01'), std::byte('\x01'),
                                    std::byte('\x00')};
  if (v != v_expected) IC_API::trap(std::string(__func__) + ": 5a");
}
{
  std::string s = "w3gef-eqbai";
  CandidTypePrincipal p = CandidTypePrincipal(s);
  std::vector<std::byte> v = p.get_M().vec();
  std::vector<std::byte> v_expected{std::byte('\x01'), std::byte('\x02'),
                                    std::byte('\x01'), std::byte('\x02')};
  if (v != v_expected) IC_API::trap(std::string(__func__) + ": 5b");
}

return 0;
}