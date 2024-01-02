// Main entry point for a native debug executable.
// Build it with: `icpp build-native` from the parent folder where 'icpp.toml' resides

#include "main.h"

#include <iostream>

#include "../src/canister.h"
#include "../src/http_incalls.h"
#include "../src/http_outcalls.h"

// The Mock IC
#include "icpp_hooks.h"
#include "mock_ic.h"

int main() {
  bool exit_on_fail = true;
  MockIC mockIC(exit_on_fail);

  // The pretend principals of the caller
  std::string my_principal{
      "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae"};
  std::string anonymous_principal{"2vxsx-fae"};

  bool silent_on_trap = true;

  // -----------------------------------------------------------------------------
  // During deployment to IC, this is called automatically. It allocates OP memory
  // '()' -> canister_init does not return directly, so skip validation check
  mockIC.run_test("canister_init", canister_init, "4449444c0000", "",
                  silent_on_trap, my_principal);

  // -----------------------------------------------------------------------------
  // '()' -> ...
  mockIC.run_test("get_icp_usd_exchange", get_icp_usd_exchange, "4449444c0000",
                  "4449444c00017104544f444f", silent_on_trap, my_principal);

  // ------------------------------------------------------------------------
  // You can call the canister with curl, and it returns a JSON:
  //
  //   LOCAL
  //     $ curl http://localhost:$(dfx info webserver-port)/counter?canisterId=$(dfx canister id my_canister)
  //     {"counter":0}
  //
  //     $ curl -X POST http://localhost:$(dfx info webserver-port)/counter?canisterId=$(dfx canister id my_canister)
  //     {"counter":1}
  //
  //   IC MAINNET
  //     $ curl https://$(dfx canister --ic id my_canister).raw.icp0.io/counter
  //    or
  //     $ curl https://<canister-id>.raw.icp0.io/counter
  //
  // ------------------------------------------------------------------------
  // http_request: GET counter
  //
  // To get the hex string of Candid used by the IC to call the http_request
  // method, outcomment the IC_API::trap statement, and issue the above shown
  // `curl` command.
  //
  // The IC ends up calling the canister's http_request method with this candid:
  //   (
  //     record {
  //         url = "/counter?canisterId=bkyz2-fmaaa-aaaaa-qaaaq-cai";
  //         method = "GET";
  //         body = vec "{}";
  //         headers = vec {
  //         record { "host"; "localhost:4943" };
  //         record { "user-agent"; "curl/7.81.0" };
  //         record { "accept"; "*/*" };
  //         };
  //         certificate_version = opt (2 : nat16);
  //     },
  //   )
  // -> The canister's http_request method returns this candid to the IC:
  // (
  //   record {
  //     body = blob "{\22counter\22:0}";
  //     headers = vec {
  //       record { "Content-Type"; "application/json" };
  //       record { "Content-Length"; "13" };
  //     };
  //     upgrade = opt false;
  //     status_code = 200 : nat16;
  //   },
  // )
  // -> The IC then returns the data as a response to the client.
  //    This final step is not tested here.
  //    It is tested in the smoketests with pytest & requests.
  //
  mockIC.run_test(
      "http_request GET counter", http_request,
      "4449444c056c05efd6e40271e1edeb4a71a2f5ed880401c6a4a1980602b0f1b99806046d7b6d036c02007101716e7a01002f2f636f756e7465723f63616e697374657249643d626b797a322d666d6161612d61616161612d71616161712d63616903474554000304686f73740e6c6f63616c686f73743a343934330a757365722d6167656e740b6375726c2f372e38312e3006616363657074032a2f2a010200",
      "4449444c0a6c02000101016d716c006c02007101716d036c02007101716c02007101716c04a2f5ed880408c6a4a19806049ce9c69906099aa1b2f90c7a6d7b6e7e01070d7b22636f756e746572223a307d020c436f6e74656e742d54797065106170706c69636174696f6e2f6a736f6e0e436f6e74656e742d4c656e6774680231330100c800",
      silent_on_trap, my_principal);

  // ------------------------------------------------------------------------
  // http_request: POST counter
  //
  // To get the hex string of Candid used by the IC to call the http_request
  // method, outcomment the IC_API::trap statement, and issue the above shown
  // `curl -X POST` command.
  //
  // The IC ends up calling the canister's http_request method with this candid:
  //   (
  //     record {
  //         url = "/counter?canisterId=bkyz2-fmaaa-aaaaa-qaaaq-cai";
  //         method = "POST";
  //         body = vec "{}";
  //         headers = vec {
  //         record { "host"; "localhost:4943" };
  //         record { "user-agent"; "curl/7.81.0" };
  //         record { "accept"; "*/*" };
  //         };
  //         certificate_version = opt (2 : nat16);
  //     },
  //   )
  // -> The canister's http_request method returns this candid to the IC:
  // (
  //   record {
  //     body = blob "{\22counter\22:0}";
  //     headers = vec {
  //       record { "Content-Type"; "application/json" };
  //       record { "Content-Length"; "13" };
  //     };
  //     upgrade = opt false;
  //     status_code = 200 : nat16;
  //   },
  // )
  // -> The IC then calls the http_request_update method (next test)
  //
  mockIC.run_test(
      "http_request POST counter", http_request,
      "4449444c056c05efd6e40271e1edeb4a71a2f5ed880401c6a4a1980602b0f1b99806046d7b6d036c02007101716e7a01002f2f636f756e7465723f63616e697374657249643d626b797a322d666d6161612d61616161612d71616161712d63616904504f5354000304686f73740e6c6f63616c686f73743a343934330a757365722d6167656e740b6375726c2f372e38312e3006616363657074032a2f2a010200",
      "4449444c086c02000101016d716c006c02007101716d036c04a2f5ed880406c6a4a19806049ce9c69906079aa1b2f90c7a6d7b6e7e010500000101cc00",
      silent_on_trap, my_principal);

  // ------------------------------------------------------------------------
  // To get the hex string of Candid used by the IC to call the http_request
  // method, outcomment the IC_API::trap statement, and issue the above shown
  // `curl -X POST` command.
  //
  // The IC ends up calling the canister's http_request_update method with this candid:
  // (
  //   record {
  //     url = "/counter?canisterId=bkyz2-fmaaa-aaaaa-qaaaq-cai";
  //     method = "POST";
  //     body = vec {};
  //     headers = vec {
  //       record { "host"; "localhost:4943" };
  //       record { "user-agent"; "curl/7.81.0" };
  //       record { "accept"; "*/*" };
  //     };
  //   },
  // )
  // -> The canister's http_request_update method returns this candid to the IC:
  // (
  //   record {
  //     body = blob "{\22counter\22:1}";
  //     headers = vec {
  //       record { "Content-Type"; "application/json" };
  //       record { "Content-Length"; "13" };
  //     };
  //     status_code = 200 : nat16;
  //   },
  // )
  // -> The IC then returns the data as a response to the client.
  //    This final step is not tested here.
  //    It is tested in the smoketests with pytest & requests.
  //
  mockIC.run_test(
      "http_request_update POST counter", http_request_update,
      "4449444c046c04efd6e40271e1edeb4a71a2f5ed880401c6a4a19806026d7b6d036c020071017101002f2f636f756e7465723f63616e697374657249643d626b797a322d666d6161612d61616161612d71616161712d63616904504f5354000304686f73740e6c6f63616c686f73743a343934330a757365722d6167656e740b6375726c2f372e38312e3006616363657074032a2f2a",
      "4449444c096c02000101016d716c006c02007101716d036c02007101716c02007101716c03a2f5ed880408c6a4a19806049aa1b2f90c7a6d7b01070d7b22636f756e746572223a317d020c436f6e74656e742d54797065106170706c69636174696f6e2f6a736f6e0e436f6e74656e742d4c656e677468023133c800",
      silent_on_trap, my_principal);

  // ------------------------------------------------------------------------
  // returns 1 if any tests failed
  return mockIC.test_summary();
}
