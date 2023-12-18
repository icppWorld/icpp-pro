#include "my_canister.h"

#include <string>

#include "ic_api.h"

#include <json/json.hpp>

/* ---------------------------------------------------------
  Use the nlohmann/json library, to create a JSON based API

  $ dfx canister call my_canister greet_json '("{\"name\": \"AJ\"}")'
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

  // Extract the "name" from the json object
  std::string name;
  if (j_in.contains("name")) {
    // there is an entry with key "name"
    name = j_in["name"];
  } else {
    IC_API::trap("ERROR in JSON: - missing key 'name' ");
  }

  // Store a greeting in a json object
  nlohmann::json j_out;
  j_out["greet"] = "Hello " + name + "!";

  // Return the json object as a Candid Text
  std::string s = j_out.dump();
  ic_api.to_wire(CandidTypeText{s});
}