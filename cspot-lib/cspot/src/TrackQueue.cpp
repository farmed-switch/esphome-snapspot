#include "TrackQueue.h"
#include <pb_decode.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>

#ifdef ESP_PLATFORM
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

#include "AccessKeyFetcher.h"
#include "BellTask.h"
#include "CDNAudioFile.h"
#include "CSpotContext.h"
#include "HTTPClient.h"
#include "Logger.h"
#include "Utils.h"
#include "WrappedSemaphore.h"
#ifdef BELL_ONLY_CJSON
#include "cJSON.h"
#else
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#endif
#include "protobuf/metadata.pb.h"

using namespace cspot;
namespace TrackDataUtils {
bool countryListContains(char* countryList, const char* country) {
  uint16_t countryList_length = strlen(countryList);
  for (int x = 0; x < countryList_length; x += 2) {
    if (countryList[x] == country[0] && countryList[x + 1] == country[1]) {
      return true;
    }
  }
  return false;
}

bool doRestrictionsApply(Restriction* restrictions, int count,
                         const char* country) {
  for (int x = 0; x < count; x++) {
    if (restrictions[x].countries_allowed != nullptr) {
      return !countryListContains(restrictions[x].countries_allowed, country);
    }

    if (restrictions[x].countries_forbidden != nullptr) {
      return countryListContains(restrictions[x].countries_forbidden, country);
    }
  }

  return false;
}

bool canPlayTrack(Track& trackInfo, int altIndex, const char* country) {
  if (altIndex < 0) {

  } else {
    for (int x = 0; x < trackInfo.alternative[altIndex].restriction_count;
         x++) {
      if (trackInfo.alternative[altIndex].restriction[x].countries_allowed !=
          nullptr) {
        return countryListContains(
            trackInfo.alternative[altIndex].restriction[x].countries_allowed,
            country);
      }

      if (trackInfo.alternative[altIndex].restriction[x].countries_forbidden !=
          nullptr) {
        return !countryListContains(
            trackInfo.alternative[altIndex].restriction[x].countries_forbidden,
            country);
      }
    }
  }
  return true;
}
}

void TrackInfo::loadPbTrack(Track* pbTrack, const std::vector<uint8_t>& gid) {

  trackId = bytesToHexString(gid);

  name = std::string(pbTrack->name);

  if (pbTrack->artist_count > 0) {

    artist = std::string(pbTrack->artist[0].name);
  }

  if (pbTrack->has_album) {

    album = std::string(pbTrack->album.name);

    if (pbTrack->album.has_cover_group &&
        pbTrack->album.cover_group.image_count > 0) {
      auto imageId =
          pbArrayToVector(pbTrack->album.cover_group.image[0].file_id);
      imageUrl = "https://i.scdn.co/image/" + bytesToHexString(imageId);
    }
  }

  number = pbTrack->has_number ? pbTrack->number : 0;
  discNumber = pbTrack->has_disc_number ? pbTrack->disc_number : 0;
  duration = pbTrack->duration;
}

void TrackInfo::loadPbEpisode(Episode* pbEpisode,
                              const std::vector<uint8_t>& gid) {

  trackId = bytesToHexString(gid);

  name = std::string(pbEpisode->name);

  if (pbEpisode->covers->image_count > 0) {

    auto imageId = pbArrayToVector(pbEpisode->covers->image[0].file_id);
    imageUrl = "https://i.scdn.co/image/" + bytesToHexString(imageId);
  }

  number = pbEpisode->has_number ? pbEpisode->number : 0;
  discNumber = 0;
  duration = pbEpisode->duration;
}

QueuedTrack::QueuedTrack(TrackReference& ref,
                         std::shared_ptr<cspot::Context> ctx,
                         uint32_t requestedPosition)
    : requestedPosition(requestedPosition), ctx(ctx) {
  this->ref = ref;

  loadedSemaphore = std::make_shared<bell::WrappedSemaphore>();
  state = State::QUEUED;
}

QueuedTrack::~QueuedTrack() {
  state = State::FAILED;
  loadedSemaphore->give();

  if (pendingMercuryRequest != 0) {
    ctx->session->unregister(pendingMercuryRequest);
  }

  if (pendingAudioKeyRequest != 0) {
    ctx->session->unregisterAudioKey(pendingAudioKeyRequest);
  }
}

std::shared_ptr<cspot::CDNAudioFile> QueuedTrack::getAudioFile() {
  if (state != State::READY) {
    return nullptr;
  }

  return std::make_shared<cspot::CDNAudioFile>(cdnUrl, audioKey);
}

void QueuedTrack::stepParseMetadata(Track* pbTrack, Episode* pbEpisode) {
  int alternativeCount, filesCount = 0;
  bool canPlay = false;
  AudioFile* selectedFiles = nullptr;

  const char* countryCode = ctx->config.countryCode.c_str();

  if (ref.type == TrackReference::Type::TRACK) {
    CSPOT_LOG(info, "Track name: %s", pbTrack->name);
    CSPOT_LOG(info, "Track duration: %d", pbTrack->duration);

    CSPOT_LOG(debug, "trackInfo.restriction.size() = %d",
              pbTrack->restriction_count);

    if (TrackDataUtils::doRestrictionsApply(
            pbTrack->restriction, pbTrack->restriction_count, countryCode)) {

      for (int x = 0; x < pbTrack->alternative_count; x++) {
        if (!TrackDataUtils::doRestrictionsApply(
                pbTrack->alternative[x].restriction,
                pbTrack->alternative[x].restriction_count, countryCode)) {
          selectedFiles = pbTrack->alternative[x].file;
          filesCount = pbTrack->alternative[x].file_count;
          trackId = pbArrayToVector(pbTrack->alternative[x].gid);
          break;
        }
      }
    } else {

      selectedFiles = pbTrack->file;
      filesCount = pbTrack->file_count;
      trackId = pbArrayToVector(pbTrack->gid);
    }

    if (trackId.size() > 0) {

      trackInfo.loadPbTrack(pbTrack, trackId);
    }
  } else {

    CSPOT_LOG(info, "Episode name: %s", pbEpisode->name);
    CSPOT_LOG(info, "Episode duration: %d", pbEpisode->duration);

    CSPOT_LOG(debug, "episodeInfo.restriction.size() = %d",
              pbEpisode->restriction_count);

    if (!TrackDataUtils::doRestrictionsApply(pbEpisode->restriction,
                                             pbEpisode->restriction_count,
                                             countryCode)) {
      selectedFiles = pbEpisode->file;
      filesCount = pbEpisode->file_count;
      trackId = pbArrayToVector(pbEpisode->gid);

      trackInfo.loadPbEpisode(pbEpisode, trackId);
    }
  }

  CSPOT_LOG(info, "=== Audio Format Selection ===");
  CSPOT_LOG(info, "Requested format: %d, Available files: %d", ctx->config.audioFormat, filesCount);
  CSPOT_LOG(info, "Country code: '%s'", countryCode);

  for (int x = 0; x < filesCount; x++) {
    CSPOT_LOG(info, "  File[%d]: format=%d", x, selectedFiles[x].format);
    if (selectedFiles[x].format == ctx->config.audioFormat) {
      fileId = pbArrayToVector(selectedFiles[x].file_id);
      CSPOT_LOG(info, "✓ Using requested format: %d", selectedFiles[x].format);
      break;
    }
  }

  if (fileId.size() == 0) {
    CSPOT_LOG(debug, "Requested format not available, trying OGG Vorbis fallbacks...");

    for (int x = 0; x < filesCount; x++) {
      if (selectedFiles[x].format == AudioFormat_OGG_VORBIS_320) {
        fileId = pbArrayToVector(selectedFiles[x].file_id);
        CSPOT_LOG(info, "Fallback to OGG Vorbis 320kbps");
        break;
      }
    }
  }

  if (fileId.size() == 0) {

    for (int x = 0; x < filesCount; x++) {
      if (selectedFiles[x].format == AudioFormat_OGG_VORBIS_160) {
        fileId = pbArrayToVector(selectedFiles[x].file_id);
        CSPOT_LOG(info, "Fallback to OGG Vorbis 160kbps");
        break;
      }
    }
  }

  if (fileId.size() == 0) {

    for (int x = 0; x < filesCount; x++) {
      if (selectedFiles[x].format == AudioFormat_OGG_VORBIS_96) {
        fileId = pbArrayToVector(selectedFiles[x].file_id);
        CSPOT_LOG(info, "Fallback to OGG Vorbis 96kbps");
        break;
      }
    }
  }

  if (fileId.size() == 0 && filesCount > 0) {
    fileId = pbArrayToVector(selectedFiles[0].file_id);
    CSPOT_LOG(info, "Using first available format: %d (last resort)", selectedFiles[0].format);
  }

  if (fileId.size() == 0) {
    CSPOT_LOG(info, "File not available for playback");

    state = State::FAILED;
    loadedSemaphore->give();
    return;
  }

  identifier = bytesToHexString(fileId);

  CSPOT_LOG(info, "Selected fileId (hex): %s (length: %zu bytes)", identifier.c_str(), fileId.size());

  state = State::KEY_REQUIRED;
}

void QueuedTrack::stepLoadAudioFile(
    std::mutex& trackListMutex,
    std::shared_ptr<bell::WrappedSemaphore> updateSemaphore) {

  this->pendingAudioKeyRequest = ctx->session->requestAudioKey(
      trackId, fileId,
      [this, &trackListMutex, updateSemaphore](
          bool success, const std::vector<uint8_t>& audioKey) {
        std::scoped_lock lock(trackListMutex);

        if (success) {
          CSPOT_LOG(info, "Got audio key");
          this->audioKey =
              std::vector<uint8_t>(audioKey.begin() + 4, audioKey.end());

          state = State::CDN_REQUIRED;
        } else {
          CSPOT_LOG(error, "Failed to get audio key");
          state = State::FAILED;
          loadedSemaphore->give();
        }
        updateSemaphore->give();
      });

  state = State::PENDING_KEY;
}

void QueuedTrack::stepLoadCDNUrl(const std::string& accessKey) {
  if (accessKey.size() == 0) {

    return;
  }

  if (retry_count > 0) {
    uint64_t now_ms = esp_timer_get_time() / 1000;
    uint64_t elapsed_ms = now_ms - last_retry_time_ms;

    uint64_t backoff_ms = 5000 * (1 << (retry_count - 1));

    if (elapsed_ms < backoff_ms) {

      uint64_t remaining_ms = backoff_ms - elapsed_ms;
      if (remaining_ms % 5000 < 100) {
        CSPOT_LOG(info, "Rate limit backoff: %llu seconds remaining (retry %d/%d)",
                  remaining_ms / 1000, retry_count, MAX_RETRIES);
      }

      vTaskDelay(pdMS_TO_TICKS(100));
      return;
    }

    CSPOT_LOG(info, "Retry attempt %d/%d after %llu ms backoff",
              retry_count + 1, MAX_RETRIES, elapsed_ms);
  }

  CSPOT_LOG(info, "Fetching CDN URL...");

  try {

    std::string fileIdHex = bytesToHexString(fileId);

    std::string requestUrl = string_format(
        "https://gew4-spclient.spotify.com/storage-resolve/files/audio/interactive/"
        "%s?version=10000000&product=9&platform=39&alt=json",
        fileIdHex.c_str());

    if (accessKey.empty()) {
      CSPOT_LOG(error, "Access token is EMPTY");
      state = State::FAILED;
      loadedSemaphore->give();
      return;
    }

    auto req = bell::HTTPClient::get(
        requestUrl, {bell::HTTPClient::ValueHeader(
                        {"Authorization", "Bearer " + accessKey})});

    if (!req) {
      CSPOT_LOG(error, "HTTPClient::get() returned nullptr");

      retry_count++;
      last_retry_time_ms = esp_timer_get_time() / 1000;

      if (retry_count >= MAX_RETRIES) {
        CSPOT_LOG(error, "Max retries (%d) exceeded - marking track as FAILED", MAX_RETRIES);
        state = State::FAILED;
        loadedSemaphore->give();
      } else {
        CSPOT_LOG(error, "Will retry after backoff (attempt %d/%d)", retry_count, MAX_RETRIES);

      }
      return;
    }

    std::string_view result = req->body();

    if (result.empty()) {
      CSPOT_LOG(error, "HTTP response body is empty from api.spotify.com (rate limit or error)");

      retry_count++;
      last_retry_time_ms = esp_timer_get_time() / 1000;

      if (retry_count >= MAX_RETRIES) {
        CSPOT_LOG(error, "Max retries (%d) exceeded - marking track as FAILED", MAX_RETRIES);
        state = State::FAILED;
        loadedSemaphore->give();
      } else {
        CSPOT_LOG(error, "Empty response - will retry after backoff (attempt %d/%d)",
                  retry_count, MAX_RETRIES);

      }
      return;
    }

    CSPOT_LOG(debug, "Received response (%zu bytes)", result.size());

#ifdef BELL_ONLY_CJSON
    cJSON* jsonResult = cJSON_Parse(result.data());
    if (!jsonResult) {
      CSPOT_LOG(error, "Failed to parse JSON response");
      state = State::FAILED;
      loadedSemaphore->give();
      return;
    }

    cJSON* cdnUrlArray = cJSON_GetObjectItem(jsonResult, "cdnurl");
    if (!cdnUrlArray || !cJSON_IsArray(cdnUrlArray) || cJSON_GetArraySize(cdnUrlArray) == 0) {
      CSPOT_LOG(error, "Invalid JSON: 'cdnurl' array missing or empty");

      cJSON_Delete(jsonResult);

      retry_count++;
      last_retry_time_ms = esp_timer_get_time() / 1000;

      if (retry_count >= MAX_RETRIES) {
        CSPOT_LOG(error, "Max retries (%d) exceeded - marking track as FAILED", MAX_RETRIES);
        state = State::FAILED;
        loadedSemaphore->give();
      } else {
        CSPOT_LOG(error, "No CDN URLs - will retry after backoff (attempt %d/%d)",
                  retry_count, MAX_RETRIES);

      }
      return;
    }

    cJSON* firstUrl = cJSON_GetArrayItem(cdnUrlArray, 0);
    if (!firstUrl || !cJSON_IsString(firstUrl)) {
      CSPOT_LOG(error, "Invalid JSON: first cdnurl entry is not a string");
      cJSON_Delete(jsonResult);
      state = State::FAILED;
      loadedSemaphore->give();
      return;
    }

    cdnUrl = firstUrl->valuestring;
    cJSON_Delete(jsonResult);
#else

    auto jsonResult = nlohmann::json::parse(result, nullptr, false);

    if (jsonResult.is_discarded()) {
      CSPOT_LOG(error, "Failed to parse JSON response from api.spotify.com");
      state = State::FAILED;
      loadedSemaphore->give();
      return;
    }

    if (!jsonResult.contains("cdnurl") || !jsonResult["cdnurl"].is_array() ||
        jsonResult["cdnurl"].empty()) {
      CSPOT_LOG(error, "Invalid JSON: 'cdnurl' array missing or empty");
      state = State::FAILED;
      loadedSemaphore->give();
      return;
    }

    cdnUrl = jsonResult["cdnurl"][0];
#endif

    CSPOT_LOG(info, "Received CDN URL (%zu chars)", cdnUrl.size());

    retry_count = 0;
    last_retry_time_ms = 0;

    state = State::READY;
    loadedSemaphore->give();
  } catch (const std::exception& ex) {
    CSPOT_LOG(error, "Exception in stepLoadCDNUrl: %s", ex.what());
    state = State::FAILED;
    loadedSemaphore->give();
  } catch (...) {
    CSPOT_LOG(error, "Unknown exception in stepLoadCDNUrl");
    state = State::FAILED;
    loadedSemaphore->give();
  }
}

void QueuedTrack::expire() {
  if (state != State::QUEUED) {
    state = State::FAILED;
    loadedSemaphore->give();
  }
}

void QueuedTrack::stepLoadMetadata(
    Track* pbTrack, Episode* pbEpisode, std::mutex& trackListMutex,
    std::shared_ptr<bell::WrappedSemaphore> updateSemaphore) {

  std::string requestUrl = string_format(
      "hm://metadata/3/%s/%s",
      ref.type == TrackReference::Type::TRACK ? "track" : "episode",
      bytesToHexString(ref.gid).c_str());

  auto responseHandler = [this, pbTrack, pbEpisode, &trackListMutex,
                          updateSemaphore](MercurySession::Response& res) {
    std::scoped_lock lock(trackListMutex);

    if (res.parts.size() == 0) {

      state = State::FAILED;
      updateSemaphore->give();
      loadedSemaphore->give();
      return;
    }

    if (ref.type == TrackReference::Type::TRACK) {
      pb_release(Track_fields, pbTrack);
      pbDecode(*pbTrack, Track_fields, res.parts[0]);
    } else {
      pb_release(Episode_fields, pbEpisode);
      pbDecode(*pbEpisode, Episode_fields, res.parts[0]);
    }

    stepParseMetadata(pbTrack, pbEpisode);

    updateSemaphore->give();
  };

  pendingMercuryRequest = ctx->session->execute(
      MercurySession::RequestType::GET, requestUrl, responseHandler);

  state = State::PENDING_META;
}

TrackQueue::TrackQueue(std::shared_ptr<cspot::Context> ctx,
                       std::shared_ptr<cspot::PlaybackState> state)
    : bell::Task("CSpotTrackQueue", 1024 * 64, 2, 0),
      playbackState(state),
      ctx(ctx) {
  accessKeyFetcher = std::make_shared<cspot::AccessKeyFetcher>(ctx);
  processSemaphore = std::make_shared<bell::WrappedSemaphore>();
  playableSemaphore = std::make_shared<bell::WrappedSemaphore>();

  playbackState->innerFrame.state.track.funcs.encode =
      &TrackReference::pbEncodeTrackList;
  playbackState->innerFrame.state.track.arg = &currentTracks;
  pbTrack = Track_init_zero;
  pbEpisode = Episode_init_zero;

  startTask();
};

TrackQueue::~TrackQueue() {
  stopTask();

  std::scoped_lock lock(tracksMutex);

  pb_release(Track_fields, &pbTrack);
  pb_release(Episode_fields, &pbEpisode);
}

TrackInfo TrackQueue::getTrackInfo(std::string_view identifier) {
  for (auto& track : preloadedTracks) {
    if (track->identifier == identifier)
      return track->trackInfo;
  }
  return TrackInfo{};
}

void TrackQueue::runTask() {
  isRunning = true;

  std::scoped_lock lock(runningMutex);

  std::deque<std::shared_ptr<QueuedTrack>> trackQueue;

  while (isRunning) {
    processSemaphore->twait(100);

    try {
      accessKey = accessKeyFetcher->getAccessKey();
    } catch (const std::exception& e) {
      CSPOT_LOG(error, "Failed to refresh access key: %s — will retry", e.what());
      vTaskDelay(pdMS_TO_TICKS(2000));
      continue;
    } catch (...) {
      CSPOT_LOG(error, "Unknown error refreshing access key — will retry");
      vTaskDelay(pdMS_TO_TICKS(2000));
      continue;
    }

    int loadedIndex = currentTracksIndex;

    if (loadedIndex < 0) {
      continue;
    } else {
      std::scoped_lock lock(tracksMutex);

      trackQueue = preloadedTracks;
    }

    for (auto& track : trackQueue) {
      if (track) {
        this->processTrack(track);
      }
    }
  }
}

void TrackQueue::stopTask() {
  if (isRunning) {
    isRunning = false;
    processSemaphore->give();
    std::scoped_lock lock(runningMutex);
  }
}

std::shared_ptr<QueuedTrack> TrackQueue::consumeTrack(
    std::shared_ptr<QueuedTrack> prevTrack, int& offset) {
  std::scoped_lock lock(tracksMutex);

  if (currentTracksIndex == -1 || currentTracksIndex >= currentTracks.size()) {
    return nullptr;
  }

  if (prevTrack == nullptr) {
    offset = 0;

    return preloadedTracks[0];
  }

  auto prevTrackIter =
      std::find(preloadedTracks.begin(), preloadedTracks.end(), prevTrack);

  if (prevTrackIter != preloadedTracks.end()) {

    offset = prevTrackIter - preloadedTracks.begin() + 1;
  } else {
    offset = 0;
  }

  if (offset >= preloadedTracks.size()) {

    return nullptr;
  }

  return preloadedTracks[offset];
}

void TrackQueue::processTrack(std::shared_ptr<QueuedTrack> track) {
  switch (track->state) {
    case QueuedTrack::State::QUEUED:
      track->stepLoadMetadata(&pbTrack, &pbEpisode, tracksMutex,
                              processSemaphore);
      break;
    case QueuedTrack::State::KEY_REQUIRED:
      track->stepLoadAudioFile(tracksMutex, processSemaphore);
      break;
    case QueuedTrack::State::CDN_REQUIRED:
      track->stepLoadCDNUrl(accessKey);

      if (track->state == QueuedTrack::State::READY) {
        if (preloadedTracks.size() < MAX_TRACKS_PRELOAD) {

          queueNextTrack(preloadedTracks.size());
        }
      }
      break;
    default:

      break;
  }
}

bool TrackQueue::queueNextTrack(int offset, uint32_t positionMs) {
  const int requestedRefIndex = offset + currentTracksIndex;

  if (requestedRefIndex < 0 || requestedRefIndex >= currentTracks.size()) {
    return false;
  }

  if (offset == 0 && preloadedTracks.size() &&
      preloadedTracks[0]->ref == currentTracks[currentTracksIndex]) {
    preloadedTracks.pop_front();
  }

  if (offset <= 0) {
    preloadedTracks.push_front(std::make_shared<QueuedTrack>(
        currentTracks[requestedRefIndex], ctx, positionMs));
  } else {
    preloadedTracks.push_back(std::make_shared<QueuedTrack>(
        currentTracks[requestedRefIndex], ctx, positionMs));
  }

  return true;
}

bool TrackQueue::skipTrack(SkipDirection dir, bool expectNotify) {
  bool skipped = true;
  std::scoped_lock lock(tracksMutex);

  if (dir == SkipDirection::PREV) {
    uint64_t position =
        !playbackState->innerFrame.state.has_position_ms
            ? 0
            : playbackState->innerFrame.state.position_ms +
                  ctx->timeProvider->getSyncedTimestamp() -
                  playbackState->innerFrame.state.position_measured_at;

    if (currentTracksIndex > 0 && position < 3000) {
      queueNextTrack(-1);

      if (preloadedTracks.size() > MAX_TRACKS_PRELOAD) {
        preloadedTracks.pop_back();
      }

      currentTracksIndex--;
    } else {
      queueNextTrack(0);
    }
  } else {
    if (currentTracks.size() > currentTracksIndex + 1) {
      preloadedTracks.pop_front();

      if (!queueNextTrack(preloadedTracks.size() + 1)) {
        CSPOT_LOG(info, "Failed to queue next track");
      }

      currentTracksIndex++;
    } else {
      skipped = false;
    }
  }

  if (skipped) {

    playbackState->innerFrame.state.playing_track_index = currentTracksIndex;

    if (expectNotify) {

      notifyPending = true;
    }
  }

  return skipped;
}

bool TrackQueue::hasTracks() {
  std::scoped_lock lock(tracksMutex);

  return currentTracks.size() > 0;
}

bool TrackQueue::isFinished() {
  std::scoped_lock lock(tracksMutex);
  return currentTracksIndex >= currentTracks.size() - 1;
}

bool TrackQueue::updateTracks(uint32_t requestedPosition, bool initial) {
  std::scoped_lock lock(tracksMutex);
  bool cleared = true;

  currentTracks = playbackState->remoteTracks;
  currentTracksIndex = playbackState->innerFrame.state.playing_track_index;

  if (initial) {

    preloadedTracks.clear();

    if (currentTracksIndex < currentTracks.size()) {

      queueNextTrack(0, requestedPosition);
    }

    notifyPending = true;

    playableSemaphore->give();
  } else if (!preloadedTracks.empty() && preloadedTracks[0]->loading) {

    preloadedTracks.erase(preloadedTracks.begin() + 1, preloadedTracks.end());

    CSPOT_LOG(info, "Keeping current track %d", currentTracksIndex);
    queueNextTrack(1);

    cleared = false;
  } else {

    preloadedTracks.clear();

    CSPOT_LOG(info, "Re-loading current track");
    queueNextTrack(0, requestedPosition);
  }

  return cleared;
}
