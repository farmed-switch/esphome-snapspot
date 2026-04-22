#pragma once

#include <string>
#ifdef BELL_ONLY_CJSON
#include "cJSON.h"
#else
#endif

namespace cspot {
class ApResolve {
 public:
  ApResolve(std::string apOverride);

  std::string fetchFirstApAddress();

 private:
  std::string apOverride;
};
}
