#pragma once

#include <stdint.h>
#include <vector>

namespace cspot {
class TimeProvider {
 private:
  int64_t timestampDiff = 0;

 public:

  TimeProvider();

  void syncWithPingPacket(const std::vector<uint8_t>& pongPacket);

  unsigned long long getSyncedTimestamp();
};
}
