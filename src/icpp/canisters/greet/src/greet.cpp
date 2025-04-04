/*
greet: The icpp-pro getting started tutorial: build, deploy & query

Try it out in Candid UI: 
https://a4gq6-oaaaa-aaaab-qaa4q-cai.raw.icp0.io/?id=eavxv-bqaaa-aaaag-ahkrq-cai

See the Getting Started tutorial:
https://docs.icpp.world/getting-started.html

*/
#include "greet.h"
#include "hello.h" // libhello
#include "world.h" // libworld

#include <string>
#include <iostream>
#include <stdio.h>

#include "ic_api.h"

#include <json/json.hpp>

/* ---------------------------------------------------------
  Respond with a 'std::string' wrapped in a CandidTypeText
*/
void greet_0() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // Print a debug message to the dfx console window
  // - Only active when running local.
  // - Does nothing when running on mainnet
  std::cout << "Hi dfx console!" << std::endl;

  // Sends a message back over the wire, in Candid format
  ic_api.to_wire(CandidTypeText{"hello!"});
}

/* ---------------------------------------------------------
  Use static library functions to build the response
*/
void greet_0_static_lib() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  std::string msg = ns_hello::hello() + " " + ns_world::world();
  std::cout << msg << std::endl;
  ic_api.to_wire(CandidTypeText{msg});
}

/* ---------------------------------------------------------
  Respond only to an authenticated user
  Best practice interface for forward compatibility: 
    Return a single record wrapped in a variant
*/
void greet_0_auth() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  CandidTypePrincipal caller = ic_api.get_caller();
  std::string principal = caller.get_text();

  // Respond with 401 (Unauthorized code) if user is not logged in
  if (caller.is_anonymous()) {
    uint16_t status_code = 401;
    ic_api.to_wire(CandidTypeVariant{"Err", CandidTypeNat16{status_code}});
    return;
  }

  // Greet an authenticated user by their principal
  CandidTypeRecord r_out;
  r_out.append("greeting", CandidTypeText{"Hello " + principal});
  ic_api.to_wire(CandidTypeVariant{"Ok", r_out});
}

// Respond with an 'int' wrapped in a CandidTypeInt
void greet_1() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);
  ic_api.to_wire(CandidTypeInt{2023});
}

/* ---------------------------------------------------------
  Extract a 'std::string" from an incoming CandidTypeText
  Respond with an 'std::string' wrapped in a CandidTypeText
*/
void greet_2() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // Get the principal of the caller, as cryptographically verified by the IC
  CandidTypePrincipal caller = ic_api.get_caller();

  // Get the name, passed as a Candid parameter to this method
  std::string name{""};
  ic_api.from_wire(CandidTypeText{&name});

  // Create a msg, to be passed back as Candid over the wire
  std::string msg;
  msg.append("hello " + name + "!\n");
  msg.append("Your principal is: " + caller.get_text());

  // Send the response back
  ic_api.to_wire(CandidTypeText{msg});
}

/* ---------------------------------------------------------------------------
  Extract an '__int128_t' & a 'std::string" from an incoming CandidTypeRecord
  Respond with an '__int128_t' & a 'std::string" wrapped in a CandidTypeRecord
*/
void greet_3() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // Get the data from the wire
  __int128_t icpp_version{0};
  std::string OS{""};

  CandidTypeRecord r_in;
  r_in.append("icpp version", CandidTypeInt{&icpp_version});
  r_in.append("OS", CandidTypeText{&OS});
  ic_api.from_wire(r_in);

  // Do something with the input
  std::string release_details;
  release_details.append("Version = " + std::to_string(int(icpp_version)) +
                         " & ");
  release_details.append("Operating System = " + OS);

  __int128_t release_year{0};
  if (icpp_version == 1.0) {
    release_year = 2023;
  } else {
    std::string msg;
    IC_API::trap("INPUT ERROR: the Release Year is not known for " +
                 release_details);
  }

  // Send it back
  CandidTypeRecord r_out;
  r_out.append("icpp Release Details", CandidTypeText{release_details});
  r_out.append("release year", CandidTypeInt{release_year});
  ic_api.to_wire(r_out);
}

/* ------------------------------------------------------------------------------------------------
  Extract data from an incoming argument list, represented as a C++ vector of two CandidTypeRecords 
  Respond using a C++ vector of different CandidTypeXXX objects
*/
void greet_4() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  __int128_t r1_i1{0};
  __int128_t r1_i2{0};
  CandidTypeRecord r1;
  r1.append(6, CandidTypeInt{&r1_i1});
  r1.append(9, CandidTypeInt{&r1_i2});

  __int128_t r2_i1{0};
  __int128_t r2_i2{0};
  CandidTypeRecord r2;
  r2.append(7, CandidTypeInt{&r2_i1});
  r2.append(10, CandidTypeInt{&r2_i2});

  CandidArgs args_in;
  args_in.append(r1);
  args_in.append(r2);
  ic_api.from_wire(args_in);

  // Build the args_out vector
  CandidArgs args_out;
  args_out.append(CandidTypeText{"Hello!"});
  args_out.append(CandidTypeText{"Your secret numbers are:"});
  args_out.append(CandidTypeInt{r1_i1});
  args_out.append(CandidTypeInt{r1_i2});
  args_out.append(CandidTypeInt{r2_i1});
  args_out.append(CandidTypeInt{r2_i2});

  // Send it back
  ic_api.to_wire(args_out);
}

/* ---------------------------------------------------------
  Use the nlohmann/json library, to create a JSON based API

  $ dfx canister call greet greet_json '("{\"name\": \"AJ\"}")'
  ("{\"greet\":\"Hello AJ!\"}")
*/
void greet_json() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // Get the Candid Text from the wire
  std::string in{""};
  ic_api.from_wire(CandidTypeText{&in});

  // Convert it into a nlohmann/json object
  // See: https://json.nlohmann.me/api/basic_json/parse/
  nlohmann::json j_in = nlohmann::json::parse(in);

  // Store a greeting or an error in a json object
  nlohmann::json j_out;

  // Extract the "name" from the json object
  std::string name;
  if (j_in.contains("name")) {
    // there is an entry with key "name"
    name = j_in["name"];
    // Store a greeting in the json object
    j_out["greet"] = "Hello " + name + "!";
  } else {
    // Store an ERROR in the json object
    j_out["ERROR"] = "missing key 'name'. The json string received is: " + in;
  }

  // Return the json object as a Candid Text
  std::string s = j_out.dump();
  ic_api.to_wire(CandidTypeText{s});
}

/* ---------------------------------------------------------
  Write a LOG file and then read & print to the dfx console
*/
void greet_log_file() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  std::cout << "--------------------------------------" << std::endl;
  std::cout << "Write a LOG file, using fopen/fprintf" << std::endl;

  std::string file_name = "greet.log";

  std::cout << "Opening the file " << file_name << std::endl;
  FILE *log_file = fopen(file_name.c_str(), "a"); // Open in append mode
  if (log_file != nullptr) {
    fprintf(log_file, "LOG: Hi log file!\n");
    fprintf(log_file, "LOG: How are you!\n");

    std::cout << "Closing the file " << file_name << std::endl;
    fclose(log_file);

  } else {
    std::cerr << "cerr: Unable to open log file " << std::endl;
  }

  std::cout << "--------------------------------------" << std::endl;
  std::cout << "Read and print the LOG file           " << std::endl;

  log_file = fopen(file_name.c_str(), "r");
  if (log_file != nullptr) {

    std::cout << "Content of " + file_name + ": " << std::endl;

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), log_file) != nullptr) {
      std::cout << buffer;
    }

    std::cout << std::endl;

    std::cout << "Closing the file " << file_name << std::endl;
    fclose(log_file);

  } else {
    std::cerr << "cerr: Unable to open log file " << std::endl;
  }

  ic_api.to_wire();
}