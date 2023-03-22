//  Wrapper around a std::vector<std::byte>

#include "vec_bytes.h"

#include <cassert>
#include <cstring>

#include <algorithm>
#include <bit>
#include <string>

#include <limits.h>
#include <limits>
#include <sstream>

#include "ic_api.h"

#define INT_STR_SIZE (sizeof(int) * CHAR_BIT / 3 + 3)

VecBytes::VecBytes() { check_endian(); }

VecBytes::~VecBytes() {}

// TODO: use __int128_t
std::string VecBytes::int_to_hex(const int i, const std::string prefix) {
  const int LEN_STR = 80;
  char str[LEN_STR] = {'\0'};
  c_int_to_hex(str, (unsigned int)i);
  std::string s{prefix};
  return s.append(str);

  // TODO: Using stringstream causes too many globals
  //       600+, only 300 allowed   (Dec 15, 2022)
  // From: https://stackoverflow.com/a/5100745/5480536
  // std::stringstream stream;
  // stream << prefix
  //        << std::setfill('0') << std::setw(2)
  //        << std::hex << i;
  // return stream.str();
}

std::string VecBytes::byte_to_hex(const std::byte b, const std::string prefix) {
  return int_to_hex(int(b), prefix);
}

// TODO: just use char ?
std::string VecBytes::int_to_dec(const int i, const int width) {
  char str[INT_STR_SIZE] = {'\0'};
  c_int_to_dec(str, INT_STR_SIZE, i);
  std::string s;
  for (size_t i = 0; i < (width - strlen(str)); ++i) {
    s.append(" ");
  }
  s.append(str);
  return std::string{s};

  // TODO: Using stringstream causes too many globals
  //       600+, only 300 allowed   (Dec 15, 2022)
  // stream << std::setfill(' ') << std::setw(width)
  //        << std::dec << i;
  // return stream.str();
}

std::string VecBytes::byte_to_char(const std::byte b) {
  return std::string{(const char)(int)b};
}

// TODO: use __int128_t
std::string VecBytes::byte_to_dec(const std::byte b, const int width) {
  return int_to_dec(int(b), width);
}

void VecBytes::c_int_to_hex(char *str, unsigned int x) {
  char arr[17] = "0123456789abcdef";
  char *p = (char *)&x;
  int j = 0;
  bool found_non_zero = false;
  for (size_t i = 0; i < 8; ++i) {
    unsigned char ch = (x >> ((32 - 4) - i * 4)) & 0xF;
    if (found_non_zero || i > 5) {

      str[j] = arr[ch];

      if (str[j] != '0') {
        found_non_zero = true;
        if (i < 6) {
          IC_API::trap(
              "PROGRAM ERROR in int_to_hex. Hex string has more than 2 "
              "characters?");
        }
      }
      ++j;
    }
  }
  str[j] = '\0';
}

// https://stackoverflow.com/a/36392305/5480536
// Convert unsigned int to dec string, without using sprintf
void VecBytes::c_int_to_dec(char *str, size_t size, int x) {
  char buf[INT_STR_SIZE];
  char *p = &buf[INT_STR_SIZE - 1];
  *p = '\0';
  int i = x;

  do {
    *(--p) = abs(i % 10) + '0';
    i /= 10;
  } while (i);

  if (x < 0) {
    *(--p) = '-';
  }
  size_t len = (size_t)(&buf[INT_STR_SIZE] - p);
  if (len > size) {
    IC_API::trap(
        "PROGRAM ERROR in c_int_to_dec - Buffer to small to store decimal "
        "representation of int");
  }
  memcpy(str, p, len);
}

void VecBytes::append_didl() {
  append_byte((std::byte)'D');
  append_byte((std::byte)'I');
  append_byte((std::byte)'D');
  append_byte((std::byte)'L');
}

// ------------------------------------------------------------------------------
// append_uleb128

// append bool as uleb128
void VecBytes::append_uleb128(const bool &v) {
  if (v) append_uleb128(__uint128_t(1));
  else append_uleb128(__uint128_t(0));
}

// append __uint128_t as uleb128
void VecBytes::append_uleb128(const __uint128_t &v) {
  uint8_t Bytes[1024];
  auto numBytes = encode_uleb128(v, Bytes);
  assert(numBytes < sizeof(Bytes));
  for (size_t i = 0; i < numBytes; ++i) {
    append_byte((std::byte)Bytes[i]);
  }
}

// ------------------------------------------------------------------------------
// append_sleb128

// append __int128_t as sleb128
void VecBytes::append_sleb128(const __int128_t &v) {
  uint8_t Bytes[1024];
  auto numBytes = encode_sleb128(v, Bytes);
  assert(numBytes < sizeof(Bytes));
  for (size_t i = 0; i < numBytes; ++i) {
    append_byte((std::byte)Bytes[i]);
  }
}

// Decode with uleb128, starting at & updating offset
bool VecBytes::parse_uleb128(__uint128_t &offset, __uint128_t &v,
                             __uint128_t &numbytes, std::string &parse_error) {
  __uint128_t len = m_vec.size() - offset;

  uint8_t *buf = &m_vec_uint8_t[offset];
  uint8_t *buf_end = &m_vec_uint8_t[offset + len];

  parse_error = "";
  v = decode_uleb128(buf, &numbytes, buf_end, parse_error);
  offset += numbytes;

  if (parse_error.size() > 0) {
    return true;
  }

  return false;
}

// Decode bytes with sleb128, starting at & updating offset
bool VecBytes::parse_sleb128(__uint128_t &offset, __int128_t &v,
                             __uint128_t &numbytes, std::string &parse_error) {
  __uint128_t len = m_vec.size() - offset;

  uint8_t *buf = &m_vec_uint8_t[offset];
  uint8_t *buf_end = &m_vec_uint8_t[offset + len];

  parse_error = "";
  v = decode_sleb128(buf, &numbytes, buf_end, parse_error);
  offset += numbytes;

  if (parse_error.size() > 0) {
    return true;
  }

  return false;
}

// Parse n bytes, starting at & updating offset
bool VecBytes::parse_bytes(__uint128_t &offset, std::vector<std::byte> &v,
                           __uint128_t &n, __uint128_t &numbytes,
                           std::string &parse_error) {
  parse_error = "";
  numbytes = 0;

  __uint128_t len = m_vec.size() - offset;

  if (n > len) {
    parse_error =
        "Not enough bytes left. The remaining bytes in the byte stream on wire is ";
    parse_error.append(VecBytes::my_uint128_to_string(len));
    parse_error.append(", but specified number to parse is ");
    parse_error.append(VecBytes::my_uint128_to_string(n));
    return true;
  }

  std::copy(m_vec.begin() + offset, m_vec.begin() + offset + n,
            std::back_inserter(v));

  numbytes = n;
  offset += numbytes;

  return false;
}

// Parse n bytes, starting at & updating offset
bool VecBytes::parse_bytes(__uint128_t &offset, std::vector<uint8_t> &v,
                           __uint128_t &n, __uint128_t &numbytes,
                           std::string &parse_error) {
  parse_error = "";
  numbytes = 0;

  __uint128_t len = m_vec.size() - offset;

  if (n > len) {
    parse_error =
        "Not enough bytes left. The remaining bytes in the byte stream on wire is ";
    parse_error.append(VecBytes::my_uint128_to_string(len));
    parse_error.append(", but specified number to parse is ");
    parse_error.append(VecBytes::my_uint128_to_string(n));
    return true;
  }

  std::copy(m_vec_uint8_t.begin() + offset, m_vec_uint8_t.begin() + offset + n,
            std::back_inserter(v));

  numbytes = n;
  offset += numbytes;

  return false;
}

void VecBytes::store(const uint8_t *bytes, const uint32_t num_bytes) {
  clear();
  append_bytes(bytes, num_bytes);
}

// Initializes m_vec from a bytes array string in hex format.
// candid_in example: "4449444c0001710d432b2b20446576656c6f706572"
// generated with   : $ didc encode '("C++ Developer")' -t'(text)'
void VecBytes::store_hex_string(const std::string hex_string) {
  // Get a vector of strings with the hex tokens, by splitting out every 2 chars
  std::vector<std::string> hex_tokens;
  for (size_t i = 0; i < hex_string.size(); i += 2)
    hex_tokens.push_back(hex_string.substr(i, 2));

  store_hex_tokens(hex_tokens);
}

// convert a vector of tokens into bytes and initialize m_vec with them
void VecBytes::store_hex_tokens(std::vector<std::string> hex_tokens) {
  clear();
  for (std::string s : hex_tokens) {
    int n;

    // TODO: Using istringstream causes too many globals
    //       600+, only 300 allowed   (Jan 15, 2023)
    // std::istringstream(s) >> std::hex >> n;

    // https://stackoverflow.com/a/45353247/5480536
    n = std::stoi(s, nullptr, 16);

    append_byte((const std::byte)n);
  }
}

void VecBytes::append_byte(std::byte b) {
  m_vec.push_back(b);

  // Maintain the same vector as a uint8_t type
  int n = (int)b;
  m_vec_uint8_t.push_back((uint8_t)n);
}

void VecBytes::append_bytes(const uint8_t *bytes, const uint32_t num_bytes) {
  for (size_t i = 0; i < num_bytes; ++i) {
    append_byte((const std::byte)bytes[i]);
  }
}

void VecBytes::trap_if_vec_does_not_start_with_DIDL() {
  std::string s;
  s.append(byte_to_char(m_vec[0]));
  s.append(byte_to_char(m_vec[1]));
  s.append(byte_to_char(m_vec[2]));
  s.append(byte_to_char(m_vec[3]));
  if (s != "DIDL") {
    IC_API::trap("ERROR: Message does not start with DIDL");
  }
}

// Prints the bytes of the vec in hex, dec & bits
void VecBytes::debug_print() {
  IC_API::debug_print("  hex, decimal, char");

  std::string s_hex;
  for (std::byte b : m_vec) {
    std::string s;
    s.append("   ");
    s.append(byte_to_hex(b, ""));
    s.append(", ");
    s.append(byte_to_dec(b, 8));
    s.append(", ");
    s.append(byte_to_char(b));
    s_hex.append(byte_to_hex(b, ""));

    IC_API::debug_print(s); // Print a nice table: hex, decimal, char
  }
  IC_API::debug_print(
      s_hex); // Print the whole thing in hex on one line, for didc decode !
}

void VecBytes::clear() {
  m_vec.clear();
  m_vec_uint8_t.clear();
}

// Helper function to assign values to __int128_t from strings
// https://stackoverflow.com/a/45608851/5480536
__int128_t VecBytes::atoint128_t(const std::string &in) {
  __int128_t res = 0;
  size_t i = 0;
  bool sign = false;

  if (in[i] == '-') {
    ++i;
    sign = true;
  }

  if (in[i] == '+') {
    ++i;
  }

  for (; i < in.size(); ++i) {
    const char c = in[i];
    if (not std::isdigit(c))
      IC_API::trap(std::string("Non-numeric character: ") + c);

    // TODO: implement multiplication overflow protection: https://stackoverflow.com/a/1815371/5480536
    res *= 10;

    res += c - '0';
    // https://stackoverflow.com/a/6472982/5480536
    if (res < c - '0') {
      IC_API::trap(
          std::string(
              "ERROR: Overflow - Cannot convert string to __uint128t.\n       Number is too big: ") +
          in);
    }
  }

  if (sign) {
    // TODO: implement multiplication overflow protection: https://stackoverflow.com/a/1815371/5480536
    res *= -1;
  }

  return res;
}

__uint128_t VecBytes::atouint128_t(const std::string &in) {
  __uint128_t res = 0;
  size_t i = 0;

  if (in[i] == '+') {
    ++i;
  }

  for (; i < in.size(); ++i) {
    const char c = in[i];
    if (not std::isdigit(c))
      IC_API::trap(std::string("Non-numeric character: ") + c);

    // TODO: implement multiplication overflow protection: https://stackoverflow.com/a/1815371/5480536
    res *= 10;

    res += c - '0';
    // https://stackoverflow.com/a/6472982/5480536
    if (res < c - '0') {
      IC_API::trap(
          std::string(
              "ERROR: Overflow - Cannot convert string to __uint128t.\n       Number is too big: ") +
          in);
    }
  }

  return res;
}

// Utility function to encode a SLEB128 value to a buffer.
// Returns the length in bytes of the encoded value.
unsigned VecBytes::encode_sleb128(__int128_t Value, uint8_t *p) {
  uint8_t *orig_p = p;
  unsigned Count = 0;
  bool More;
  do {
    uint8_t Byte = Value & 0x7f;
    // NOTE: this assumes that this signed shift is an arithmetic right shift.
    Value >>= 7;
    More = !((((Value == 0) && ((Byte & 0x40) == 0)) ||
              ((Value == -1) && ((Byte & 0x40) != 0))));
    Count++;
    if (More)
      Byte |= 0x80; // Mark this byte to show that more bytes will follow.
    *p++ = Byte;
  } while (More);
  return (unsigned)(p - orig_p);
}

// Utility function to encode a ULEB128 value to a buffer.
// Returns the length in bytes of the encoded value.
unsigned VecBytes::encode_uleb128(__uint128_t Value, uint8_t *p) {
  uint8_t *orig_p = p;
  unsigned Count = 0;
  do {
    uint8_t Byte = Value & 0x7f;
    Value >>= 7;
    Count++;
    if (Value != 0)
      Byte |= 0x80; // Mark this byte to show that more bytes will follow.
    *p++ = Byte;
  } while (Value != 0);
  return (unsigned)(p - orig_p);
}

// Utility function to decode a ULEB128 value.
__uint128_t VecBytes::decode_uleb128(const uint8_t *p, __uint128_t *n,
                                     const uint8_t *end, std::string &error) {
  const uint8_t *orig_p = p;
  __uint128_t Value = 0;
  unsigned Shift = 0;
  do {
    if (p == end) {
      error.append("ERROR: malformed uleb128, extends past end");
      *n = (unsigned)(p - orig_p);
      return 0;
    }
    __uint128_t Slice = *p & 0x7f;
    if ((Shift >= 128 && Slice != 0) || Slice << Shift >> Shift != Slice) {
      error.append("uleb128 too big for __uint128_t");
      *n = (unsigned)(p - orig_p);
      return 0;
    }
    Value += Slice << Shift;
    Shift += 7;
  } while (*p++ >= 128);
  *n = (unsigned)(p - orig_p);
  return Value;
}

// Utility function to decode a SLEB128 value.
__int128_t VecBytes::decode_sleb128(const uint8_t *p, __uint128_t *n,
                                    const uint8_t *end, std::string &error) {
  const uint8_t *orig_p = p;
  // TODO: update this function for an __int128_t
  int64_t Value = 0;
  unsigned Shift = 0;
  uint8_t Byte;
  do {
    if (p == end) {
      error.append("malformed sleb128, extends past end");
      if (n) *n = (unsigned)(p - orig_p);
      return 0;
    }
    Byte = *p;
    uint64_t Slice = Byte & 0x7f;
    if ((Shift >= 64 && Slice != (Value < 0 ? 0x7f : 0x00)) ||
        (Shift == 63 && Slice != 0 && Slice != 0x7f)) {
      error.append("sleb128 too big for int64");
      if (n) *n = (unsigned)(p - orig_p);
      return 0;
    }
    Value |= Slice << Shift;
    Shift += 7;
    ++p;
  } while (Byte >= 128);
  // Sign extend negative numbers if needed.
  if (Shift < 64 && (Byte & 0x40)) Value |= (-1ULL) << Shift;
  if (n) *n = (unsigned)(p - orig_p);
  return __int128_t(Value);
}

std::string VecBytes::my_uint128_to_string(__uint128_t num) {
  // https://stackoverflow.com/a/55970931/5480536
  std::string str;
  do {
    int digit = num % 10;
    str = std::to_string(digit) + str;
    num = (num - digit) / 10;
  } while (num != 0);
  return str;
}

// Utility function to get the size of the ULEB128-encoded value.
unsigned VecBytes::getULEB128Size(uint64_t Value) {
  unsigned Size = 0;
  do {
    Value >>= 7;
    Size += sizeof(int8_t);
  } while (Value);
  return Size;
}

// Utility function to get the size of the SLEB128-encoded value.
unsigned VecBytes::getSLEB128Size(int64_t Value) {
  unsigned Size = 0;
  int Sign = Value >> (8 * sizeof(Value) - 1);
  bool IsMore;

  do {
    unsigned Byte = Value & 0x7f;
    Value >>= 7;
    IsMore = Value != Sign || ((Byte ^ Sign) & 0x40) != 0;
    Size += sizeof(int8_t);
  } while (IsMore);
  return Size;
}

// Utility function to check if we are running on little-endian
bool VecBytes::is_little_endian() {
  if constexpr (std::endian::native == std::endian::little) return true;
  else return false;
}

// Utility function to check if we are running on big-endian
bool VecBytes::is_big_endian() {
  if constexpr (std::endian::native == std::endian::big) return true;
  else return false;
}

// Utility function to check if we are running on mixed-endian
bool VecBytes::is_mixed_endian() {
  if (is_little_endian() || is_big_endian()) return false;
  else return true;
}

// Utility function to check if we are running on a system with IEEE 754 floats
bool VecBytes::is_float_ieee754() {
  if constexpr (std::numeric_limits<double>::is_iec559 == true) return true;
  else return false;
}

void VecBytes::check_endian() {
  if (is_little_endian()) m_endian_type = "little";
  else if (is_big_endian()) {
    m_endian_type = "big";
  } else if (is_mixed_endian()) {
    m_endian_type = "mixed";
  } else {
    IC_API::trap("Could not determine endianness of architecture. ");
  }
}

void VecBytes::trap(const std::string &msg) { IC_API::trap(msg); }