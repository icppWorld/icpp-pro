// Candid Assert helper class

#pragma once

#include <string>
#include <vector>

#include "candid.h"

class CandidAssert {
public:
  CandidAssert();
  ~CandidAssert();

  static int assert_candid(const VecBytes &B,
                           const std::string &candid_expected,
                           const bool &assert_value);
};