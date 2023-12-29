// Demo of http incalls, so you can call the canister as a regular Web2.0 HTTP server
#include "canister.h"
#include "http_incalls.h"

#include <algorithm>
#include <memory>
#include <string>
#include <variant>

#include "ic_api.h"

#include <json/json.hpp>

// URL helper function
std::string strip_query_parameters(const std::string &url) {
  size_t pos = url.find('?');
  if (pos != std::string::npos) {
    return url.substr(0, pos);
  }
  return url;
}

/*
 GET  /counter -> IC calls http_request -> {"counter": value-of-counter}

 POST /counter -> IC calls http_request -> do an upgrade call (!)
               -> the IC will call http_request_update
                  -> increments the counter by 1
                  -> {"counter": value-of-counter}
*/
void http_request() {
  IC_API ic_api(CanisterQuery{std::string(__func__)}, false);

  // Outcomment this if you want to see the Candid that is sent to the canister.
  // This is how I got the hex string for the test in ../native/main.cpp
  // IC_API::debug_print("Entered http_request !");
  // IC_API::trap("Hex String from wire = " + ic_api.get_B_in().as_hex_string());

  IC_HttpRequest request;
  ic_api.from_wire(request);

  uint16_t status_code{200};
  std::optional<bool> upgrade;
  nlohmann::json j_out;
  if (strip_query_parameters(request.url) == "/counter") {
    if (request.method == "GET") {
      status_code = 200; // OK
      upgrade = false;
      // j_out["counter"] = counter;
      if (p_counter) {
        j_out["counter"] = p_counter->counter;
      }
    } else if (request.method == "POST") {
      status_code = 204; // No Content
      upgrade = true;    // Tell the IC to call http_request_update
    } else {
      status_code = 405; // Method Not Allowed
    }
  } else {
    status_code = 404; // Not Found
  }

  IC_HttpResponse response;
  response.status_code = status_code;

  if (status_code == 405) {
    IC_HeaderField allowHeader;
    allowHeader.name = "Allow";
    allowHeader.value = "GET, POST";
    response.headers.push_back(allowHeader);
  }

  if (!j_out.empty()) {
    IC_HeaderField contentTypeHeader;
    contentTypeHeader.name = "Content-Type";
    contentTypeHeader.value = "application/json";
    response.headers.push_back(contentTypeHeader);

    std::string s_out = j_out.dump();
    IC_HeaderField contentLengthHeader;
    contentLengthHeader.name = "Content-Length";
    contentLengthHeader.value = std::to_string(s_out.size());
    response.headers.push_back(contentLengthHeader);

    // store s_out in body as an std::vector<uint8_t>
    response.body.assign(s_out.data(), s_out.data() + s_out.size());
  }

  response.upgrade = upgrade;

  ic_api.to_wire(response);
}

void http_request_update() {
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);

  // Outcomment this if you want to see the Candid that is sent to the canister.
  // This is how I got the hex string for the test in ../native/main.cpp
  // IC_API::debug_print("Entered http_request_update !");
  // IC_API::trap("Hex String from wire = " + ic_api.get_B_in().as_hex_string());

  IC_HttpUpdateRequest request;
  ic_api.from_wire(request);

  uint16_t status_code{200};
  std::optional<bool> upgrade;
  nlohmann::json j_out;
  if (strip_query_parameters(request.url) == "/counter") {
    if (request.method == "POST") {
      status_code = 200; // OK
      // ++counter;
      // j_out["counter"] = counter;
      if (p_counter) {
        ++p_counter->counter;
        j_out["counter"] = p_counter->counter;
      }
    } else {
      status_code = 405; // Method Not Allowed
    }
  } else {
    status_code = 404; // Not Found
  }

  IC_HttpResponse response;
  response.status_code = status_code;

  if (status_code == 405) {
    IC_HeaderField allowHeader;
    allowHeader.name = "Allow";
    allowHeader.value = "POST";
    response.headers.push_back(allowHeader);
  }

  if (!j_out.empty()) {
    IC_HeaderField contentTypeHeader;
    contentTypeHeader.name = "Content-Type";
    contentTypeHeader.value = "application/json";
    response.headers.push_back(contentTypeHeader);

    std::string s_out = j_out.dump();
    IC_HeaderField contentLengthHeader;
    contentLengthHeader.name = "Content-Length";
    contentLengthHeader.value = std::to_string(s_out.size());
    response.headers.push_back(contentLengthHeader);

    // store s_out in body as an std::vector<uint8_t>
    response.body.assign(s_out.data(), s_out.data() + s_out.size());
  }

  response.upgrade = upgrade;

  ic_api.to_wire(response);
}