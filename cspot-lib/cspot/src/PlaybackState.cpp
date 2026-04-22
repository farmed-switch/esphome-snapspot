#include "PlaybackState.h"

#include <string.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <type_traits>
#include <utility>

#include "BellLogger.h"
#include "CSpotContext.h"
#include "ConstantParameters.h"
#include "Logger.h"
#include "NanoPBHelper.h"
#include "Packet.h"
#include "pb.h"
#include "pb_decode.h"
#include "protobuf/spirc.pb.h"

using namespace cspot;

PlaybackState::PlaybackState(std::shared_ptr<cspot::Context> ctx) {
  this->ctx = ctx;
  innerFrame = {};
  remoteFrame = {};

  remoteFrame.state.track.funcs.decode = &TrackReference::pbDecodeTrackList;
  remoteFrame.state.track.arg = &remoteTracks;

  innerFrame.ident = strdup(ctx->config.deviceId.c_str());
  innerFrame.protocol_version = strdup(protocolVersion);

  innerFrame.state.has_position_ms = true;
  innerFrame.state.position_ms = 0;

  innerFrame.state.status = PlayStatus_kPlayStatusStop;
  innerFrame.state.has_status = true;

  innerFrame.state.position_measured_at = 0;
  innerFrame.state.has_position_measured_at = true;

  innerFrame.state.shuffle = false;
  innerFrame.state.has_shuffle = true;

  innerFrame.state.repeat = false;
  innerFrame.state.has_repeat = true;

  innerFrame.device_state.sw_version = strdup(swVersion);

  innerFrame.device_state.is_active = false;
  innerFrame.device_state.has_is_active = true;

  innerFrame.device_state.can_play = true;
  innerFrame.device_state.has_can_play = true;

  innerFrame.device_state.volume = ctx->config.volume;
  innerFrame.device_state.has_volume = true;

  innerFrame.device_state.name = strdup(ctx->config.deviceName.c_str());

  addCapability(CapabilityType_kCanBePlayer, 1);
  addCapability(CapabilityType_kDeviceType, 4);
  addCapability(CapabilityType_kGaiaEqConnectId, 1);
  addCapability(CapabilityType_kSupportsLogout, 0);
  addCapability(CapabilityType_kSupportsPlaylistV2, 1);
  addCapability(CapabilityType_kIsObservable, 1);
  addCapability(CapabilityType_kVolumeSteps, 64);
  addCapability(CapabilityType_kSupportedContexts, -1,
                std::vector<std::string>({"album", "playlist", "search",
                                          "inbox", "toplist", "starred",
                                          "publishedstarred", "track"}));
  addCapability(CapabilityType_kSupportedTypes, -1,
                std::vector<std::string>(
                    {"audio/track", "audio/episode", "audio/episode+track"}));
  innerFrame.device_state.capabilities_count = 8;
}

PlaybackState::~PlaybackState() {
  pb_release(Frame_fields, &innerFrame);
  pb_release(Frame_fields, &remoteFrame);
}

void PlaybackState::setPlaybackState(const PlaybackState::State state) {
  switch (state) {
    case State::Loading:

      innerFrame.state.status = PlayStatus_kPlayStatusPause;
      innerFrame.state.position_ms = 0;
      innerFrame.state.position_measured_at =
          ctx->timeProvider->getSyncedTimestamp();
      break;
    case State::Playing:
      innerFrame.state.status = PlayStatus_kPlayStatusPlay;
      innerFrame.state.position_measured_at =
          ctx->timeProvider->getSyncedTimestamp();
      break;
    case State::Stopped:
      break;
    case State::Paused:

      innerFrame.state.status = PlayStatus_kPlayStatusPause;
      uint32_t diff = ctx->timeProvider->getSyncedTimestamp() -
                      innerFrame.state.position_measured_at;
      this->updatePositionMs(innerFrame.state.position_ms + diff);
      break;
  }
}

void PlaybackState::syncWithRemote() {
  const char* src = remoteFrame.state.context_uri ? remoteFrame.state.context_uri : "";
  char* newPtr = (char*)realloc(innerFrame.state.context_uri, strlen(src) + 1);
  if (!newPtr) {
    CSPOT_LOG(error, "syncWithRemote: realloc failed");
    return;
  }
  innerFrame.state.context_uri = newPtr;
  strcpy(innerFrame.state.context_uri, src);

  innerFrame.state.has_playing_track_index = true;
  innerFrame.state.playing_track_index = remoteFrame.state.playing_track_index;
}

bool PlaybackState::isActive() {
  return innerFrame.device_state.is_active;
}

void PlaybackState::setActive(bool isActive) {
  innerFrame.device_state.is_active = isActive;
  if (isActive) {
    innerFrame.device_state.became_active_at =
        ctx->timeProvider->getSyncedTimestamp();
    innerFrame.device_state.has_became_active_at = true;
  }
}

void PlaybackState::updatePositionMs(uint32_t position) {
  innerFrame.state.position_ms = position;
  innerFrame.state.position_measured_at =
      ctx->timeProvider->getSyncedTimestamp();
}

void PlaybackState::setVolume(uint32_t volume) {
  innerFrame.device_state.volume = volume;
  ctx->config.volume = volume;
}

bool PlaybackState::decodeRemoteFrame(std::vector<uint8_t>& data) {
  pb_release(Frame_fields, &remoteFrame);

  remoteTracks.clear();

  pbDecode(remoteFrame, Frame_fields, data);

  return true;
}

std::vector<uint8_t> PlaybackState::encodeCurrentFrame(MessageType typ) {

  innerFrame.version = 1;
  innerFrame.seq_nr = this->seqNum;
  innerFrame.typ = typ;

  if (nextStateUpdateId_ >= 0) {
    innerFrame.state_update_id = (uint64_t)nextStateUpdateId_;

  } else {
    innerFrame.state_update_id = ctx->timeProvider->getSyncedTimestamp();
  }
  innerFrame.has_version = true;
  innerFrame.has_seq_nr = true;
  innerFrame.recipient_count = 0;
  innerFrame.has_state = true;
  innerFrame.has_device_state = true;
  innerFrame.has_typ = true;
  innerFrame.has_state_update_id = true;

  CSPOT_LOG(info, "SPIRC OUT: type=%d seq=%d stateId=%lld status=%d pos=%u "
            "measuredAt=%llu active=%d vol=%d ctxUri=%s trackIdx=%d",
            (int)typ, (int)this->seqNum,
            (long long)innerFrame.state_update_id,
            (int)innerFrame.state.status,
            (unsigned)innerFrame.state.position_ms,
            (unsigned long long)innerFrame.state.position_measured_at,
            (int)innerFrame.device_state.is_active,
            (int)innerFrame.device_state.volume,
            innerFrame.state.context_uri ? innerFrame.state.context_uri : "(null)",
            innerFrame.state.has_playing_track_index
                ? (int)innerFrame.state.playing_track_index : -1);

  this->seqNum += 1;

  return pbEncode(Frame_fields, &innerFrame);
}

void PlaybackState::addCapability(CapabilityType typ, int intValue,
                                  std::vector<std::string> stringValue) {
  innerFrame.device_state.capabilities[capabilityIndex].has_typ = true;
  this->innerFrame.device_state.capabilities[capabilityIndex].typ = typ;

  if (intValue != -1) {
    this->innerFrame.device_state.capabilities[capabilityIndex].intValue[0] =
        intValue;
    this->innerFrame.device_state.capabilities[capabilityIndex].intValue_count =
        1;
  } else {
    this->innerFrame.device_state.capabilities[capabilityIndex].intValue_count =
        0;
  }

  for (int x = 0; x < stringValue.size(); x++) {
    pbPutString(stringValue[x],
                this->innerFrame.device_state.capabilities[capabilityIndex]
                    .stringValue[x]);
  }

  this->innerFrame.device_state.capabilities[capabilityIndex]
      .stringValue_count = stringValue.size();

  this->capabilityIndex += 1;
}
