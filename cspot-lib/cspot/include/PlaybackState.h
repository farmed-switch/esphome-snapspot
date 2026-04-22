#pragma once

#include <stdint.h>
#include <memory>
#include <string>
#include <vector>

#include "TrackReference.h"
#include "protobuf/spirc.pb.h"

namespace cspot {
struct Context;

class PlaybackState {
 private:
  std::shared_ptr<cspot::Context> ctx;

  uint32_t seqNum = 0;
  uint8_t capabilityIndex = 0;

  std::vector<uint8_t> frameData;

  void addCapability(
      CapabilityType typ, int intValue = -1,
      std::vector<std::string> stringsValue = std::vector<std::string>());

 public:
  Frame innerFrame;
  Frame remoteFrame;

  std::vector<TrackReference> remoteTracks;

  int64_t nextStateUpdateId_ = -1;

  enum class State { Playing, Stopped, Loading, Paused };

  PlaybackState(std::shared_ptr<cspot::Context> ctx);

  ~PlaybackState();

  void setPlaybackState(const PlaybackState::State state);

  void setActive(bool isActive);

  bool isActive();

  void updatePositionMs(uint32_t position);

  void setVolume(uint32_t volume);

  void syncWithRemote();

  std::vector<uint8_t> encodeCurrentFrame(MessageType typ);

  bool decodeRemoteFrame(std::vector<uint8_t>& data);
};
}
