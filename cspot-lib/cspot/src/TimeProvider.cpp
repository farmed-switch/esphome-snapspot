#include "TimeProvider.h"

#include "BellLogger.h"
#include "Logger.h"
#include "Utils.h"
#ifndef _WIN32
#include <arpa/inet.h>
#endif

using namespace cspot;

TimeProvider::TimeProvider() {}

void TimeProvider::syncWithPingPacket(const std::vector<uint8_t>& pongPacket) {

  if (pongPacket.size() < 4) {
    CSPOT_LOG(error, "syncWithPingPacket: packet too short (%zu bytes)", pongPacket.size());
    return;
  }

  int64_t remoteTimestamp =
      (int64_t)(((uint64_t)ntohl(extract<uint32_t>(pongPacket, 0))) * 1000);
  int64_t localTimestamp = (int64_t)getCurrentTimestamp();
  this->timestampDiff = remoteTimestamp - localTimestamp;

}

unsigned long long TimeProvider::getSyncedTimestamp() {

  int64_t synced = (int64_t)getCurrentTimestamp() + this->timestampDiff;
  return synced > 0 ? (unsigned long long)synced : 0;
}