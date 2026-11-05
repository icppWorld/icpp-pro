// Re-encodes Candid messages with icpp-candid's encoder, for
// scripts/candid_repin.py: one message in hex per line on stdin, one line
// out per message, "OK <hex>" or "ERR <why>".
//
// Each message is decoded at its own types, as its header gives them, and
// encoded again. The type table comes out as the encoder writes it: in Rust's
// order, without entries no argument uses, equal types shared.

#include <iostream>
#include <string>
#include <vector>

#include "candid_decoder.h"
#include "candid_encoder.h"
#include "ic0.h"

static bool unhex(const std::string &s, std::vector<uint8_t> &out) {
  if (s.size() % 2) return false;
  for (size_t i = 0; i < s.size(); i += 2) {
    unsigned v;
    if (sscanf(s.c_str() + i, "%2x", &v) != 1) return false;
    out.push_back(uint8_t(v));
  }
  return true;
}

static std::string reencode(const std::string &hex) {
  std::vector<uint8_t> m;
  if (!unhex(hex, m)) return "ERR not hex";
  CandidTypeGraph w;
  std::vector<int> args;
  std::vector<CandidValue> values;
  std::string err;
  if (!CandidDecoder::read_types(m.data(), m.size(), w, args, err) ||
      !CandidDecoder::decode(m.data(), m.size(), w, args, values, err))
    return "ERR " + err;
  std::vector<uint8_t> out;
  if (!CandidEncoder::encode(w, args, values, out, err)) return "ERR " + err;
  static const char *digits = "0123456789abcdef";
  std::string s = "OK ";
  for (uint8_t b : out) {
    s.push_back(digits[b >> 4]);
    s.push_back(digits[b & 15]);
  }
  return s;
}

int main() {
  std::string line;
  while (std::getline(std::cin, line)) {
    try {
      std::cout << reencode(line) << "\n";
    } catch (const FuzzTrap &e) {
      std::cout << "ERR trap: " << e.what() << "\n";
    }
  }
  return 0;
}
