// Constants for the Mock IC

#pragma once

#include <string>

constexpr std::string_view MOCKIC_CANISTER_SELF = "5ugrv-zqaaa-aaaag-acfna-cai";
constexpr __uint128_t MOCKIC_INITIAL_CYCLES_BALANCE = 3'000'000'000'000ULL;
constexpr std::string_view MOCKIC_CONTROLLER =
    "expmt-gtxsw-inftj-ttabj-qhp5s-nozup-n3bbo-k7zvn-dg4he-knac3-lae";

// A canister that runs code has been installed at least once.
constexpr uint64_t MOCKIC_INITIAL_CANISTER_VERSION = 1;
constexpr uint32_t MOCKIC_CANISTER_STATUS_RUNNING = 1;
// A real 13-node application subnet on mainnet.
constexpr std::string_view MOCKIC_SUBNET_SELF =
    "pzp6e-ekpqk-3c5x7-2h6so-njoeq-mt45d-h3h6c-q3mxf-vpeq5-fk5o7-yae";
constexpr uint32_t MOCKIC_SUBNET_NODE_COUNT = 13;
// The IC mainnet root key: a DER-encoded BLS12-381 public key (133 bytes).
constexpr std::string_view MOCKIC_ROOT_KEY_HEX =
    "308182301d060d2b0601040182dc7c0503010201060c2b0601040182dc7c0503020103"
    "6100814c0e6ec71fab583b08bd81373c255c3c371b2e84863c98a4f1e08b74235d14fb"
    "5d9c0cd546d9685f913a0c0b2cc5341583bf4b4392e467db96d65b9bb4cb717112f847"
    "2e0d5a4d14505ffd7484b01291091c5f87b98883463f98091a0baaae";

// Cycle costs on a 13-node application subnet, as the local replica reports
// them. The IC changes its fees over time; these are for native testing.
constexpr uint64_t MOCKIC_COST_CALL_BASE = 42'102'412'000;
constexpr uint64_t MOCKIC_COST_CALL_PER_BYTE = 1'000;
constexpr uint64_t MOCKIC_COST_CREATE_CANISTER = 500'000'000'000;
constexpr uint64_t MOCKIC_COST_HTTP_REQUEST_BASE = 49'140'000;
constexpr uint64_t MOCKIC_COST_HTTP_REQUEST_PER_REQUEST_BYTE = 5'200;
constexpr uint64_t MOCKIC_COST_HTTP_REQUEST_PER_RESPONSE_BYTE = 10'400;
// cost_http_request_v2 (pay-as-you-go): per millisecond of round trip and per
// transform instruction; a non-replicated outcall costs 1/13 (one node of 13)
constexpr uint64_t MOCKIC_COST_HTTP_REQUEST_PER_MS = 1'000;
constexpr uint64_t MOCKIC_COST_HTTP_REQUEST_PER_TRANSFORM_INSTRUCTION = 1;
constexpr uint64_t MOCKIC_SUBNET_SIZE = MOCKIC_SUBNET_NODE_COUNT;
// Threshold signing & vetKD: the production key, and the test keys.
constexpr uint64_t MOCKIC_COST_SIGN_KEY_1 = 26'153'846'153;
constexpr uint64_t MOCKIC_COST_SIGN_TEST_KEY = 10'000'000'000;
