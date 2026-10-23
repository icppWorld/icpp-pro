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
// ---------------------------------------------------------------------------
// HTTPS outcalls - see IC_Management::http_request

// docs start: http_outcall
enum class IC_HttpMethod { Get, Head, Post, Put, Delete, Patch };

struct IC_HttpOutcallRequest {
  std::string url;
  IC_HttpMethod method{IC_HttpMethod::Get};
  std::vector<IC_HeaderField> headers;
  std::optional<std::vector<uint8_t>> body;
  // The largest response the canister accepts, and pays for (default and
  // maximum: 2 MB)
  std::optional<uint64_t> max_response_bytes;
  // A query of this canister that the response passes through on every node,
  // e.g. to remove headers that differ per node, so the nodes agree on it
  std::optional<std::string> transform_method;
  std::vector<uint8_t> transform_context;
  // false: the request is sent by one node instead of every node of the
  // subnet. Cheaper, but the response is not agreed on by consensus.
  std::optional<bool> is_replicated;
  // 2: pay-as-you-go pricing, where the cycles not used are refunded; 1 or
  // empty: the original pricing
  std::optional<uint32_t> pricing_version;
};

struct IC_HttpOutcallResponse {
  __uint128_t status{0};
  std::vector<IC_HeaderField> headers;
  std::vector<uint8_t> body;
};

// The argument of a transform query
struct IC_HttpTransformArgs {
  IC_HttpOutcallResponse response;
  std::vector<uint8_t> context;
}; // docs end: http_outcall
