// Unit tests for ic/vendors

#include "unit_test_vendors.h"

#include <algorithm>
#include <string>

#include "ZipIterator.hpp"
#include "ic_api.h"

void test_ZipIterator() {
  { // https://github.com/dpellegr/ZipIterator/blob/master/main.cpp
    std::vector<int> a{3, 1, 4, 2};
    std::vector<std::string> b{"Alice", "Bob", "Charles", "David"};

    std::vector<int> a_sorted{1, 2, 3, 4};
    std::vector<std::string> b_sorted{"Bob", "David", "Alice", "Charles"};

    auto zip = Zip(a, b);
    std::sort(zip.begin(), zip.end());
    if (a != a_sorted || b != b_sorted) IC_API::trap("test_ZipIterator: 1");
  }
}

int unit_test_vendors() {
  test_ZipIterator();
  return 0;
}