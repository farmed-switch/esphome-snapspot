#include "SpircHandler.h"

#include <cstring>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

#include "BellLogger.h"
#include "CSpotContext.h"
#include "Logger.h"
#include "MercurySession.h"
#include "NanoPBHelper.h"
#include "Packet.h"
#include "PlaybackState.h"
#include "TrackPlayer.h"
#include "TrackQueue.h"
#include "TrackReference.h"
#include "Utils.h"
#include "pb_decode.h"
#include "protobuf/spirc.pb.h"

using namespace cspot;

SpircHandler::SpircHandler(std::shared_ptr<cspot::Context> ctx) {
  this->playbackState = std::make_shared<PlaybackState>(ctx);
  this->trackQueue = std::make_shared<cspot::TrackQueue>(ctx, playbackState);

  auto EOFCallback = [this]() {

    ownershipProtected_ = false;

    if (trackQueue->isFinished()) {
      sendEvent(EventType::DEPLETED);
    }
  };

  auto trackLoadedCallback = [this](std::shared_ptr<QueuedTrack> track,
                                    bool paused = false) {

    ownershipProtected_ = false;

    playbackState->setPlaybackState(paused ? PlaybackState::State::Paused
                                           : PlaybackState::State::Playing);
    playbackState->updatePositionMs(track->requestedPosition);

    playbackState->nextStateUpdateId_ = lastLoadStateId_;
    this->notify();

    sendEvent(EventType::PLAYBACK_START, (int)track->requestedPosition);
    sendEvent(EventType::PLAY_PAUSE, paused);
  };

  this->ctx = ctx;
  this->trackPlayer = std::make_shared<TrackPlayer>(
      ctx, trackQueue, EOFCallback, trackLoadedCallback);

  ctx->session->setConnectedHandler([this]() {

    this->lastLoadStateId_ = 0;
    this->subscribeToMercury();
  });
}

void SpircHandler::subscribeToMercury() {
  auto responseLambda = [this](MercurySession::Response& res) {
    if (res.fail)
      return;

    sendCmd(MessageType_kMessageTypeHello);
    CSPOT_LOG(info, "Sent kMessageTypeHello! (our deviceId=%s, user=%s)",
             this->ctx->config.deviceId.c_str(),
             this->ctx->config.username.c_str());

    this->ctx->config.countryCode = this->ctx->session->getCountryCode();
  };
  auto subscriptionLambda = [this](MercurySession::Response& res) {
    if (res.fail)
      return;
    CSPOT_LOG(debug, "Received subscription response");

    this->handleFrame(res.parts[0]);
  };

  ctx->session->executeSubscription(
      MercurySession::RequestType::SUB,
      "hm://remote/user/" + ctx->config.username + "/", responseLambda,
      subscriptionLambda);
}

void SpircHandler::notifyAudioEnded() {
  playbackState->updatePositionMs(0);
  notify();
  trackPlayer->resetState(true);
}

void SpircHandler::notifyAudioReachedPlayback() {
  int offset = 0;

  auto currentTrack = trackQueue->consumeTrack(nullptr, offset);

  if (trackQueue->notifyPending) {
    trackQueue->notifyPending = false;

    playbackState->updatePositionMs(currentTrack->requestedPosition);

    currentTrack->requestedPosition = 0;
  } else {
    trackQueue->skipTrack(TrackQueue::SkipDirection::NEXT, false);
    playbackState->updatePositionMs(0);

    currentTrack = trackQueue->consumeTrack(nullptr, offset);
  }

  this->notify();

  sendEvent(EventType::TRACK_INFO, currentTrack->trackInfo);
}

void SpircHandler::updatePositionMs(uint32_t position) {
  playbackState->updatePositionMs(position);
  notify();
}

void SpircHandler::disconnect() {
  playbackState->nextStateUpdateId_ = -1;

  this->ctx->session->disconnect();
  this->trackQueue->stopTask();
  this->trackPlayer->stop();
}

void SpircHandler::handleFrame(std::vector<uint8_t>& data) {

  playbackState->decodeRemoteFrame(data);

  bool fromOtherDevice = false;
  if (playbackState->remoteFrame.ident &&
      std::strlen(playbackState->remoteFrame.ident) > 0) {
    fromOtherDevice =
        std::strcmp(playbackState->remoteFrame.ident,
                    ctx->config.deviceId.c_str()) != 0;
  }

  bool remoteActive =
      playbackState->remoteFrame.has_device_state &&
      playbackState->remoteFrame.device_state.has_is_active &&
      playbackState->remoteFrame.device_state.is_active;

  CSPOT_LOG(info, "SPIRC frame: type=%d ident=%s fromOther=%d remoteActive=%d weActive=%d ownerProt=%d stateId=%lld",
            (int)playbackState->remoteFrame.typ,
            (playbackState->remoteFrame.ident && std::strlen(playbackState->remoteFrame.ident) > 0)
                ? playbackState->remoteFrame.ident : "(no-ident)",
            (int)fromOtherDevice,
            (int)remoteActive,
            (int)playbackState->isActive(),
            (int)ownershipProtected_,
            playbackState->remoteFrame.has_state_update_id
                ? (long long)playbackState->remoteFrame.state_update_id : -1LL);

  if (playbackState->remoteFrame.has_state_update_id) {
    playbackState->nextStateUpdateId_ =
        (int64_t)playbackState->remoteFrame.state_update_id;
  }

  if (playbackState->isActive() && fromOtherDevice && remoteActive) {
    if (ownershipProtected_) {
      CSPOT_LOG(error, "SPIRC: DISC blocked by ownershipProtected (ident=%s type=%d)",
                (playbackState->remoteFrame.ident && std::strlen(playbackState->remoteFrame.ident) > 0)
                    ? playbackState->remoteFrame.ident : "(no-ident)",
                (int)playbackState->remoteFrame.typ);
      return;
    }
    CSPOT_LOG(error, "SPIRC: DISC! ident=%s type=%d fromOther=%d remoteActive=%d weActive=%d ownerProt=%d",
              (playbackState->remoteFrame.ident && std::strlen(playbackState->remoteFrame.ident) > 0)
                  ? playbackState->remoteFrame.ident : "(no-ident)",
              (int)playbackState->remoteFrame.typ,
              (int)fromOtherDevice,
              (int)remoteActive,
              (int)playbackState->isActive(),
              (int)ownershipProtected_);
    playbackState->nextStateUpdateId_ = -1;
    playbackState->setActive(false);
    this->trackPlayer->stop();
    sendEvent(EventType::DISC);
    return;
  }

  switch (playbackState->remoteFrame.typ) {
    case MessageType_kMessageTypeNotify: {
      CSPOT_LOG(info, "Notify frame: ident=%s weActive=%d remoteActive=%d",
                (playbackState->remoteFrame.ident && std::strlen(playbackState->remoteFrame.ident) > 0)
                    ? playbackState->remoteFrame.ident : "(no-ident)",
                (int)playbackState->isActive(),
                (int)remoteActive);
      break;
    }
    case MessageType_kMessageTypeSeek: {
      this->trackPlayer->seekMs(playbackState->remoteFrame.position);

      playbackState->updatePositionMs(playbackState->remoteFrame.position);

      notify();

      sendEvent(EventType::SEEK, (int)playbackState->remoteFrame.position);
      break;
    }
    case MessageType_kMessageTypeVolume:
      playbackState->setVolume(playbackState->remoteFrame.volume);
      this->notify();
      sendEvent(EventType::VOLUME, (int)playbackState->remoteFrame.volume);
      break;
    case MessageType_kMessageTypePause:
      setPause(true);
      break;
    case MessageType_kMessageTypePlay:
      setPause(false);
      break;
    case MessageType_kMessageTypeNext:
      if (nextSong()) {
        sendEvent(EventType::NEXT);
      }
      break;
    case MessageType_kMessageTypePrev:
      if (previousSong()) {
        sendEvent(EventType::PREV);
      }
      break;
    case MessageType_kMessageTypeLoad: {

      if (playbackState->isActive() &&
          playbackState->remoteFrame.has_state_update_id &&
          lastLoadStateId_ > 0 &&
          (int64_t)playbackState->remoteFrame.state_update_id <= lastLoadStateId_) {
        CSPOT_LOG(error,
                  "SPIRC: Ignoring replayed Load (stateId=%lld <= last=%lld, "
                  "ident=%s fromOther=%d)",
                  (long long)playbackState->remoteFrame.state_update_id,
                  (long long)lastLoadStateId_,
                  (playbackState->remoteFrame.ident &&
                   std::strlen(playbackState->remoteFrame.ident) > 0)
                      ? playbackState->remoteFrame.ident : "(no-ident)",
                  (int)fromOtherDevice);
        notify();
        break;
      }

      lastLoadStateId_ = playbackState->remoteFrame.has_state_update_id
                             ? (int64_t)playbackState->remoteFrame.state_update_id
                             : 0;

      this->trackPlayer->start();

      CSPOT_LOG(error, "Load frame %d tracks! ident=%s fromOther=%d weActive=%d ownerProt=%d",
                (int)playbackState->remoteTracks.size(),
                (playbackState->remoteFrame.ident && std::strlen(playbackState->remoteFrame.ident) > 0)
                    ? playbackState->remoteFrame.ident : "(no-ident)",
                (int)fromOtherDevice,
                (int)playbackState->isActive(),
                (int)ownershipProtected_);

      if (playbackState->remoteTracks.size() == 0) {
        CSPOT_LOG(info, "No tracks in frame, stopping playback");
        break;
      }

      ownershipProtected_ = true;

      playbackState->setActive(true);

      playbackState->updatePositionMs(playbackState->remoteFrame.position);
      playbackState->setPlaybackState(PlaybackState::State::Playing);

      playbackState->syncWithRemote();

      trackQueue->updateTracks(playbackState->remoteFrame.position, true);

      if (playbackState->remoteFrame.has_state_update_id) {
        playbackState->nextStateUpdateId_ =
            (int64_t)playbackState->remoteFrame.state_update_id;
      }
      this->notify();

      trackPlayer->resetState();
      break;
    }
    case MessageType_kMessageTypeReplace: {
      CSPOT_LOG(debug, "Got replace frame %d",
                playbackState->remoteTracks.size());
      playbackState->syncWithRemote();

      bool cleared = trackQueue->updateTracks(
          playbackState->remoteFrame.state.position_ms +
              ctx->timeProvider->getSyncedTimestamp() -
              playbackState->innerFrame.state.position_measured_at,
          false);

      this->notify();

      if (cleared) {
        sendEvent(EventType::FLUSH);
        trackPlayer->resetState();
      }
      break;
    }
    case MessageType_kMessageTypeShuffle: {
      CSPOT_LOG(debug, "Got shuffle frame");
      this->notify();
      break;
    }
    case MessageType_kMessageTypeRepeat: {
      CSPOT_LOG(debug, "Got repeat frame");
      this->notify();
      break;
    }
    case MessageType_kMessageTypeGoodbye: {

      if (!fromOtherDevice) {
        CSPOT_LOG(error, "SPIRC: Goodbye from server -- stopping playback");
        playbackState->nextStateUpdateId_ = -1;
        playbackState->setActive(false);
        ownershipProtected_ = false;
        this->trackPlayer->stop();
        sendEvent(EventType::DISC);
      } else {
        CSPOT_LOG(info, "SPIRC: Goodbye from other device %s -- ignored",
                  (playbackState->remoteFrame.ident &&
                   std::strlen(playbackState->remoteFrame.ident) > 0)
                      ? playbackState->remoteFrame.ident : "(no-ident)");
      }
      break;
    }
    default:
      break;
  }
}

void SpircHandler::setRemoteVolume(int volume) {
  playbackState->setVolume(volume);
  notify();
}

void SpircHandler::notify() {
  this->sendCmd(MessageType_kMessageTypeNotify);
}

bool SpircHandler::skipSong(TrackQueue::SkipDirection dir) {
  bool skipped = trackQueue->skipTrack(dir);

  trackPlayer->resetState(!skipped);

  return skipped;
}

bool SpircHandler::nextSong() {
  return skipSong(TrackQueue::SkipDirection::NEXT);
}

bool SpircHandler::previousSong() {
  return skipSong(TrackQueue::SkipDirection::PREV);
}

std::shared_ptr<TrackPlayer> SpircHandler::getTrackPlayer() {
  return this->trackPlayer;
}

void SpircHandler::sendCmd(MessageType typ) {

  auto encodedFrame = playbackState->encodeCurrentFrame(typ);

  auto responseLambda = [=](MercurySession::Response& res) {
    if (res.fail) {
      CSPOT_LOG(error, "sendCmd failed for message type %d", (int)typ);
    }
  };
  auto parts = MercurySession::DataParts({encodedFrame});
  ctx->session->execute(MercurySession::RequestType::SEND,
                        "hm://remote/user/" + ctx->config.username + "/",
                        responseLambda, parts);
}
void SpircHandler::setEventHandler(EventHandler handler) {
  this->eventHandler = handler;
}

void SpircHandler::setPause(bool isPaused) {
  if (isPaused) {
    CSPOT_LOG(debug, "External pause command");
    playbackState->setPlaybackState(PlaybackState::State::Paused);
  } else {
    CSPOT_LOG(debug, "External play command");

    playbackState->setPlaybackState(PlaybackState::State::Playing);
  }
  notify();
  sendEvent(EventType::PLAY_PAUSE, isPaused);
}

void SpircHandler::reactivate() {

  if (playbackState->isActive()) {
    CSPOT_LOG(info, "reactivate() called but already active - ignoring");
    return;
  }

  CSPOT_LOG(info, "reactivate(): re-asserting active state after DISC");

  playbackState->setActive(true);
  playbackState->setPlaybackState(PlaybackState::State::Playing);

  notify();
}

void SpircHandler::sendEvent(EventType type) {
  auto event = std::make_unique<Event>();
  event->eventType = type;
  event->data = {};
  eventHandler(std::move(event));
}

void SpircHandler::sendEvent(EventType type, EventData data) {
  auto event = std::make_unique<Event>();
  event->eventType = type;
  event->data = data;
  eventHandler(std::move(event));
}
