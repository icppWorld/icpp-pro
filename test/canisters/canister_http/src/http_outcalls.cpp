// Demo of http outcalls, so the canister can call external Web2.0 HTTP servers
#include "canister.h"
#include "http_outcalls.h"

#include <algorithm>
#include <memory>
#include <string>
#include <variant>

#include "ic_api.h"

#include <json/json.hpp>

/*
 Reference: https://internetcomputer.org/docs/current/developer-docs/integrations/https-outcalls/https-outcalls-get

 To get today's ICP-USD exchange rate of May 1, 2023, with timestamp=1682978460:
 $ curl 'https://api.pro.coinbase.com/products/ICP-USD/candles?start=1682978460&end=1682978460&granularity=60'
 [[1682978460,5.714,5.718,5.714,5.714,243.5678]]
   timestamp ,low  ,high ,open ,close,volume

*/
void get_icp_usd_exchange() {
  // update call, because HTTP outcalls go through consensus
  IC_API ic_api(CanisterUpdate{std::string(__func__)}, false);

  // TODO !!
  //1. DECLARE MANAGEMENT CANISTER
  //You need this so you can use it to make the HTTP request
  // let ic : Types.IC = actor ("aaaaa-aa");

  //2. SETUP ARGUMENTS FOR HTTP GET request

  // 2.1 Setup the URL and its query parameters
  uint64_t start_timestamp = 1682978460; //May 1, 2023 22:01:00 GMT
  uint64_t seconds_of_time = 60;         //start with 60 seconds
  std::string host = "api.pro.coinbase.com";

  std::string url = "https://" + host + "/products/ICP-USD/candles" +
                    "?start=" + std::to_string(start_timestamp) +
                    "&end=" + std::to_string(start_timestamp) +
                    "&granularity=" + std::to_string(seconds_of_time);

  //   // 2.2 prepare headers for the system http_request call
  //   //Note that `HttpHeader` is declared in line 4
  //   let request_headers = vec![
  //       HttpHeader {
  //           name: "Host".to_string(),
  //           value: format!("{host}:443"),
  //       },
  //       HttpHeader {
  //           name: "User-Agent".to_string(),
  //           value: "exchange_rate_canister".to_string(),
  //       },
  //   ];

  // uint16_t status_code{200};
  // std::optional<bool> upgrade;
  // nlohmann::json j_out;
  // if (strip_query_parameters(request.url) == "/counter") {
  //   if (request.method == "GET") {
  //     status_code = 200; // OK
  //     upgrade = false;
  //     // j_out["counter"] = counter;
  //     if (p_counter) {
  //       j_out["counter"] = p_counter->counter;
  //     }
  //   } else if (request.method == "POST") {
  //     status_code = 204; // No Content
  //     upgrade = true;    // Tell the IC to call http_request_update
  //   } else {
  //     status_code = 405; // Method Not Allowed
  //   }
  // } else {
  //   status_code = 404; // Not Found
  // }

  // IC_HttpResponse response;
  // response.status_code = status_code;

  // if (status_code == 405) {
  //   IC_HeaderField allowHeader;
  //   allowHeader.name = "Allow";
  //   allowHeader.value = "GET, POST";
  //   response.headers.push_back(allowHeader);
  // }

  // if (!j_out.empty()) {
  //   IC_HeaderField contentTypeHeader;
  //   contentTypeHeader.name = "Content-Type";
  //   contentTypeHeader.value = "application/json";
  //   response.headers.push_back(contentTypeHeader);

  //   std::string s_out = j_out.dump();
  //   IC_HeaderField contentLengthHeader;
  //   contentLengthHeader.name = "Content-Length";
  //   contentLengthHeader.value = std::to_string(s_out.size());
  //   response.headers.push_back(contentLengthHeader);

  //   // store s_out in body as an std::vector<uint8_t>
  //   response.body.assign(s_out.data(), s_out.data() + s_out.size());
  // }

  // response.upgrade = upgrade;

  std::string response{"TODO"};
  ic_api.to_wire(CandidTypeText(response));
}
