// Unit tests for ic/vendors

#include "unit_test_vendors.h"

#include <algorithm>
#include <string>

#include "cppcodec/base32_rfc4648.hpp"
#include "hash-library/crc32.h"
#include "ic_api.h"

void test_cppcodec() {
  { // https://github.com/tplgy/cppcodec/blob/master/README.md#base32
    std::string s = "EXPMTGTXSWINFTJTTABJQHP5SNOZUPN3BBOK7ZVNDG4HEKNAC3LAE===";
    std::vector<uint8_t> bytes = cppcodec::base32_rfc4648::decode(s);
    if (bytes.size() != 33) IC_API::trap("test_cppcodec: 1a");

    std::string s2 = cppcodec::base32_rfc4648::encode(bytes);
    if (s2 != s) IC_API::trap("test_cppcodec: 1b");
  }
}

void test_hash_library() {
  // https://create.stephan-brumme.com/hash-library/
  {
    CRC32 crc32;

    // hash a string
    std::string s = "123456789";
    std::string h = crc32(s);
    // get the hash as uint8_t bytes
    std::array<uint8_t, 4> hb;
    crc32.getHash(hb.data());

    // check it
    if (h != "cbf43926") IC_API::trap("test_hash_library: 1a");

    std::array<uint8_t, 4> hb_expected{uint8_t('\xcb'), uint8_t('\xf4'),
                                       uint8_t('\x39'), uint8_t('\x26')};
    if (hb != hb_expected) IC_API::trap("test_hash_library: 1b");
  }

  {
    CRC32 crc32;

    // hash an std::array of <uint8_t>
    std::array<uint8_t, 4> bytes{uint8_t('\xcb'), uint8_t('\xf4'),
                                 uint8_t('\x39'), uint8_t('\x26')};
    std::string h = crc32(bytes.data(), bytes.size());
    // get the hash as uint8_t bytes
    std::array<uint8_t, 4> hb;
    crc32.getHash(hb.data());

    // check it
    if (h != "ee4c6550") IC_API::trap("test_hash_library: 2a");

    std::array<uint8_t, 4> hb_expected{uint8_t('\xee'), uint8_t('\x4c'),
                                       uint8_t('\x65'), uint8_t('\x50')};
    if (hb != hb_expected) IC_API::trap("test_hash_library: 1b");
  }
}

int unit_test_vendors() {
  test_cppcodec();
  test_hash_library();
  return 0;
}