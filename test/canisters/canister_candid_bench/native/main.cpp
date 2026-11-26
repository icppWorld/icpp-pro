// Runs every benchmark method once through the Mock IC, with small values.
// Build it with `icpp build-native`, run it with `icpp run-native`.
//
// Natively, ic0.performance_counter counts nanoseconds, so this checks only
// that each method decodes or encodes and replies the right size. The
// instruction counts come from the deployed canister (test/test_apis.py).

#include "main.h"

#include <cstdint>
#include <string>

#include "../src/my_canister.h"

#include "mock_ic.h"

int main() {
  MockIC mockIC(true);
  // A counter that stays at 0 keeps the replies the same on every run
  mockIC.set_performance_counter([](uint32_t) -> uint64_t { return 0; });
  const std::string did = "src/my_canister.did";

  // Decoding: the size is the number of payload bytes hashed
  mockIC.run_test_text(
      "decode_smtp_request", decode_smtp_request, did, "decode_smtp_request",
      "(record { message = opt record { headers = vec { record { name = "
      "\"From\"; value = \"a@b.c\" } }; body = blob \"\\01\\02\\03\" }; "
      "envelope = null; gateway_flags = null; message_id = null })",
      "size = 12 : nat64");
  mockIC.run_test_text(
      "decode_mail_send", decode_mail_send, did, "decode_mail_send",
      "(record { client_id = \"c1\"; to = vec { \"x@y.z\" }; cc = vec {}; bcc "
      "= vec {}; subject = \"Hi\"; html_body = \"<p>\"; attachments = vec { "
      "record { filename = \"a.txt\"; content = blob \"abcd\" } } })",
      "size = 21 : nat64");
  mockIC.run_test_text("decode_blob", decode_blob, did, "decode_blob",
                       "(blob \"abcdef\")", "size = 6 : nat64");
  mockIC.run_test_text("decode_text", decode_text, did, "decode_text",
                       "(\"hello\")", "size = 5 : nat64");
  mockIC.run_test_text("decode_vec_nat64", decode_vec_nat64, did,
                       "decode_vec_nat64", "(vec { 1; 2; 3 })",
                       "size = 24 : nat64");
  mockIC.run_test_text(
      "decode_vec_record10", decode_vec_record10, did, "decode_vec_record10",
      "(vec { record { f0 = 1; f1 = 2; f2 = 3; f3 = 4; f4 = 5; f5 = \"a\"; f6 "
      "= \"bb\"; f7 = \"\"; f8 = \"c\"; f9 = \"d\" } })",
      "size = 45 : nat64");

  // Encoding: the size is the number of bytes of the encoded message, the
  // same as didc's encoding of the value (test/test_apis.py)
  mockIC.run_test_text("encode_mail_list", encode_mail_list, did,
                       "encode_mail_list", "(2 : nat64)", "size = 632 : nat64");
  mockIC.run_test_text("encode_mail_get", encode_mail_get, did,
                       "encode_mail_get", "(100 : nat64)",
                       "size = 533 : nat64");
  mockIC.run_test_text("encode_mail_export_chunk", encode_mail_export_chunk,
                       did, "encode_mail_export_chunk", "(100 : nat64)",
                       "size = 146 : nat64");
  mockIC.run_test_text("encode_blob", encode_blob, did, "encode_blob",
                       "(100 : nat64)", "size = 110 : nat64");
  mockIC.run_test_text("encode_text", encode_text, did, "encode_text",
                       "(100 : nat64)", "size = 108 : nat64");
  mockIC.run_test_text("encode_vec_nat64", encode_vec_nat64, did,
                       "encode_vec_nat64", "(10 : nat64)", "size = 90 : nat64");
  mockIC.run_test_text("encode_vec_record10", encode_vec_record10, did,
                       "encode_vec_record10", "(3 : nat64)",
                       "size = 283 : nat64");
  mockIC.run_test_text("encode_record", encode_record, did, "encode_record",
                       "(10 : nat64)", "size = 129 : nat64");
  mockIC.run_test_text("encode_small", encode_small, did, "encode_small",
                       "(7 : nat64)", "size = 55 : nat64");

  return mockIC.test_summary();
}
