// High level interface for the HTTP Gateway Protocol & HTTP Outcalls
// https://internetcomputer.org/docs/current/references/http-gateway-protocol-spec#canister-http-interface
// https://internetcomputer.org/docs/current/references/http-gateway-protocol-spec
// https://internetcomputer.org/docs/current/references/ic-interface-spec/#ic-candid
// https://internetcomputer.org/docs/current/developer-docs/integrations/https-outcalls

#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

struct IC_HeaderField {
  std::string name;
  std::string value;
};

struct IC_HttpRequest {
  // Representing 'method' and 'url' as std::string
  std::string method;
  std::string url;

  // Representing 'headers' as a vector of HeaderField
  std::vector<IC_HeaderField> headers;

  // Representing 'body' as a vector of uint8_t (blob)
  std::vector<uint8_t> body;

  // Representing 'certificate_version' as std::optional<uint16_t>
  std::optional<uint16_t> certificate_version;
};

struct IC_HttpUpdateRequest {
  // Representing 'method' and 'url' as std::string
  std::string method;
  std::string url;

  // Representing 'headers' as a vector of HeaderField
  std::vector<IC_HeaderField> headers;

  // Representing 'body' as a vector of uint8_t (blob)
  std::vector<uint8_t> body;
};

struct IC_HttpResponse {
  // Representing 'status_code' as uint16_t
  uint16_t status_code = 200;

  // Representing 'headers' as a vector of HeaderField
  std::vector<IC_HeaderField> headers;

  // Representing 'body' as a vector of uint8_t (blob)
  std::vector<uint8_t> body;

  // Representing 'upgrade' as std::optional<bool>
  std::optional<bool> upgrade;
};