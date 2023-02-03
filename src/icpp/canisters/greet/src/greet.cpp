// C++ example canister

#include "greet.h"

#include <string>

#include "ic_api.h"

void greet_0() {
  IC_API ic_api(false);
  ic_api.to_wire(CandidTypeText("hello!"));
}

void greet_1() {
  IC_API ic_api(false);
  ic_api.to_wire(CandidTypeInt(2023));
}

void greet_2() {
  IC_API ic_api(false);

  std::string name{""};
  ic_api.from_wire(CandidTypeText(&name));

  ic_api.to_wire(CandidTypeText("hello " + name + "!"));
}

void greet_3() {
  IC_API ic_api(false);

  // ---------------------------------------------------------------------------
  // Get the data from the wire
  __int128_t icpp_version{0};
  std::string OS{""};

  CandidTypeRecord r_in;
  r_in.append("icpp version", CandidTypeInt(&icpp_version));
  r_in.append("OS", CandidTypeText(&OS));
  //
  std::vector<CandidType> args_in;
  args_in.push_back(r_in);
  //
  ic_api.from_wire(args_in);

  // ---------------------------------------------------------------------------
  // Do something with the input
  std::string release_details;
  release_details.append("Version = " + std::to_string(int(icpp_version)) +
                         " & ");
  release_details.append("Operating System = " + OS);

  __int128_t release_year = 0;
  if (icpp_version == 1.0) {
    release_year = 2023;
  } else {
    std::string msg;
    IC_API::trap("INPUT ERROR: the Release Year is not known for " +
                 release_details);
  }

  // ---------------------------------------------------------------------------
  // Send it back
  CandidTypeRecord r_out;
  r_out.append("icpp Release Details", CandidTypeText(release_details));
  r_out.append("release year", CandidTypeInt(release_year));

  std::vector<CandidType> args_out;
  args_out.push_back(r_out);

  ic_api.to_wire(args_out);
}

void greet_4() {
  IC_API ic_api(false);

  __int128_t r1_i1{0};
  __int128_t r1_i2{0};
  CandidTypeRecord r1;

  r1.append(6, CandidTypeInt(&r1_i1));
  r1.append(9, CandidTypeInt(&r1_i2));
  //
  __int128_t r2_i1{0};
  __int128_t r2_i2{0};
  CandidTypeRecord r2;
  r2.append(7, CandidTypeInt(&r2_i1));
  r2.append(10, CandidTypeInt(&r2_i2));
  //
  std::vector<CandidType> args_in;
  args_in.push_back(r1);
  args_in.push_back(r2);
  //
  ic_api.from_wire(args_in);

  // Build the args_out vector
  std::vector<CandidType> args_out;
  args_out.push_back(CandidTypeText("Hello!"));
  args_out.push_back(CandidTypeText("Your secret numbers are:"));
  args_out.push_back(CandidTypeInt(r1_i1));
  args_out.push_back(CandidTypeInt(r1_i2));
  args_out.push_back(CandidTypeInt(r2_i1));
  args_out.push_back(CandidTypeInt(r2_i2));

  // Send it back
  ic_api.to_wire(args_out);
}