#include "TrackPlayer.h"

#include <exception>
#include <mutex>
#include <string>
#include <type_traits>
#include <vector>

#include "BellLogger.h"
#include "BellUtils.h"
#include "Logger.h"
#include "Packet.h"
#include "TrackQueue.h"
#include "WrappedSemaphore.h"

#ifdef BELL_VORBIS_FLOAT
#define VORBIS_SEEK(file, position) \
  (ov_time_seek(file, (double)position / 1000))
#define VORBIS_READ(file, buffer, bufferSize, section) \
  (ov_read(file, buffer, bufferSize, 0, 2, 1, section))
#else
#define VORBIS_SEEK(file, position) (ov_time_seek(file, position))
#define VORBIS_READ(file, buffer, bufferSize, section) \
  (ov_read(file, buffer, bufferSize, section))
#endif

namespace cspot {
struct Context;
struct TrackReference;
}

using namespace cspot;

static size_t vorbisReadCb(void* ptr, size_t size, size_t nmemb,
                           TrackPlayer* self) {
  return self->_vorbisRead(ptr, size, nmemb);
}

static int vorbisCloseCb(TrackPlayer* self) {
  return self->_vorbisClose();
}

static int vorbisSeekCb(TrackPlayer* self, int64_t offset, int whence) {

  return self->_vorbisSeek(offset, whence);
}

static long vorbisTellCb(TrackPlayer* self) {
  return self->_vorbisTell();
}

TrackPlayer::TrackPlayer(std::shared_ptr<cspot::Context> ctx,
                         std::shared_ptr<cspot::TrackQueue> trackQueue,
                         EOFCallback eof, TrackLoadedCallback trackLoaded)
#if defined(CONFIG_IDF_TARGET_ESP32S3)
    : bell::Task("cspot_player", 32 * 1024, 5, 1) {
#else
    : bell::Task("cspot_player", 12 * 1024, 5, 0) {
#endif
  this->ctx = ctx;
  this->eofCallback = eof;
  this->trackLoaded = trackLoaded;
  this->trackQueue = trackQueue;
  this->playbackSemaphore = std::make_unique<bell::WrappedSemaphore>(5);
  this->taskDoneSem_ = std::make_unique<bell::WrappedSemaphore>(1);

  vorbisFile = {};
  vorbisCallbacks = {
      (decltype(ov_callbacks::read_func))&vorbisReadCb,
      (decltype(ov_callbacks::seek_func))&vorbisSeekCb,
      (decltype(ov_callbacks::close_func))&vorbisCloseCb,
      (decltype(ov_callbacks::tell_func))&vorbisTellCb,
  };
}

TrackPlayer::~TrackPlayer() {
  isRunning = false;
  resetState();
  if (taskStarted_.exchange(false)) {
    taskDoneSem_->wait();
  }
}

void TrackPlayer::start() {
  if (!isRunning) {
    isRunning = true;
    taskStarted_ = true;
    startTask();
  }
}

void TrackPlayer::stop() {
  isRunning = false;
  pendingReset = true;
  currentSongPlaying = false;

  if (taskStarted_.exchange(false)) {
    taskDoneSem_->wait();
  }
}

void TrackPlayer::resetState(bool paused) {

  this->pendingReset = true;
  this->currentSongPlaying = false;
  this->startPaused = paused;
  CSPOT_LOG(info, "Resetting state");
}

void TrackPlayer::seekMs(size_t ms) {
  if (inFuture) {

    resetState();
  }

  CSPOT_LOG(info, "Seeking...");
  this->pendingSeekPositionMs = ms;
}

void TrackPlayer::runTask() {
  std::shared_ptr<QueuedTrack> track, newTrack = nullptr;

  int trackOffset = 0;
  bool eof = false;
  bool endOfQueueReached = false;

  while (isRunning) {

    if (!this->trackQueue->hasTracks() ||
        (!pendingReset && endOfQueueReached && trackQueue->isFinished())) {
      this->trackQueue->playableSemaphore->twait(300);
      continue;
    }

    if (pendingReset) {
      track = nullptr;
      pendingReset = false;
      inFuture = false;
    }

    endOfQueueReached = false;

    BELL_SLEEP_MS(50);

    if (pendingReset) {
      continue;
    }

    newTrack = trackQueue->consumeTrack(track, trackOffset);

    if (newTrack == nullptr) {
      if (trackOffset == -1) {

        track = nullptr;
      }

      BELL_SLEEP_MS(100);
      continue;
    }

    track = newTrack;

    inFuture = trackOffset > 0;

    if (track->state != QueuedTrack::State::READY) {
      track->loadedSemaphore->twait(5000);

      if (track->state != QueuedTrack::State::READY) {
        CSPOT_LOG(error, "Track failed to load, skipping it");

        CSPOT_LOG(info, "Waiting 5 seconds before next track to avoid rate limiting...");
        BELL_SLEEP_MS(5000);

        this->eofCallback();
        continue;
      }
    }

    CSPOT_LOG(info, "Got track ID=%s", track->identifier.c_str());

    currentSongPlaying = true;

    {
      std::scoped_lock lock(playbackMutex);

      {
        bool audioFileReady = false;
        for (int alloc_attempt = 0; alloc_attempt < 3; alloc_attempt++) {
          if (alloc_attempt > 0) {
            CSPOT_LOG(info, "getAudioFile() heap retry %d/3 for track %s",
                      alloc_attempt + 1, track->identifier.c_str());
            BELL_SLEEP_MS(2000);
          }
          try {
            currentTrackStream = track->getAudioFile();
            audioFileReady = true;
            break;
          } catch (const std::bad_alloc&) {
            CSPOT_LOG(error, "getAudioFile() std::bad_alloc (attempt %d/3) for track %s",
                      alloc_attempt + 1, track->identifier.c_str());
            currentTrackStream = nullptr;
          } catch (const std::exception& e) {
            CSPOT_LOG(error, "getAudioFile() failed for track %s: %s",
                      track->identifier.c_str(), e.what());
            currentTrackStream = nullptr;
            break;
          } catch (...) {
            CSPOT_LOG(error, "getAudioFile() unknown exception for track %s",
                      track->identifier.c_str());
            currentTrackStream = nullptr;
            break;
          }
        }
        if (!audioFileReady) {
          track->loading = false;
          currentSongPlaying = false;
          BELL_SLEEP_MS(1000);
          this->eofCallback();
          continue;
        }
      }

      try {
        currentTrackStream->openStream();
      } catch (const std::exception& e) {
        CSPOT_LOG(error, "openStream() failed for track %s: %s",
                  track->identifier.c_str(), e.what());
        currentTrackStream = nullptr;
        track->loading = false;
        currentSongPlaying = false;
        BELL_SLEEP_MS(2000);
        this->eofCallback();
        continue;
      } catch (...) {
        CSPOT_LOG(error, "openStream() failed with unknown exception for track %s",
                  track->identifier.c_str());
        currentTrackStream = nullptr;
        track->loading = false;
        currentSongPlaying = false;
        BELL_SLEEP_MS(2000);
        this->eofCallback();
        continue;
      }

      if (pendingReset || !currentSongPlaying) {
        continue;
      }

      int32_t r =
          ov_open_callbacks(this, &vorbisFile, NULL, 0, vorbisCallbacks);
      if (r < 0) {
        CSPOT_LOG(error, "ov_open_callbacks failed: %d for track %s",
                  r, track->identifier.c_str());
        currentTrackStream = nullptr;
        track->loading = false;
        currentSongPlaying = false;
        this->eofCallback();
        continue;
      }

      if (trackOffset == 0 && pendingSeekPositionMs == 0) {
        this->trackLoaded(track, startPaused);
        startPaused = false;
      }

      if (pendingSeekPositionMs > 0) {
        track->requestedPosition = pendingSeekPositionMs;
      }

      if (track->requestedPosition > 0) {
        VORBIS_SEEK(&vorbisFile, track->requestedPosition);
      }

      eof = false;
      track->loading = true;

      CSPOT_LOG(info, "Playing");

      while (!eof && currentSongPlaying) {

        if (pendingSeekPositionMs > 0) {
          uint32_t seekPosition = pendingSeekPositionMs;

          pendingSeekPositionMs = 0;

          VORBIS_SEEK(&vorbisFile, seekPosition);
        }

        long ret = VORBIS_READ(&vorbisFile, (char*)&pcmBuffer[0],
                               pcmBuffer.size(), &currentSection);

        if (ret == 0) {
          CSPOT_LOG(info, "EOF");

          eof = true;
        } else if (ret < 0) {
          CSPOT_LOG(error, "An error has occured in the stream %d", ret);
          currentSongPlaying = false;
        } else {
          if (this->dataCallback != nullptr) {
            auto toWrite = ret;

            while (!eof && currentSongPlaying && !pendingReset && toWrite > 0) {
              int written = 0;
              {
                std::scoped_lock dataOutLock(dataOutMutex);

                if (!currentSongPlaying || pendingReset)
                  break;

                written = dataCallback(pcmBuffer.data() + (ret - toWrite),
                                       toWrite, track->identifier);
              }
              if (written == 0) {
                BELL_SLEEP_MS(50);
              }
              toWrite -= written;
            }
          }
        }
      }
      ov_clear(&vorbisFile);

      CSPOT_LOG(info, "Playing done");

      currentTrackStream = nullptr;
      track->loading = false;
    }

    if (eof) {
      if (trackQueue->isFinished()) {
        endOfQueueReached = true;
      }

      this->eofCallback();
    }
  }

  taskDoneSem_->give();
}

size_t TrackPlayer::_vorbisRead(void* ptr, size_t size, size_t nmemb) {
  if (this->currentTrackStream == nullptr) {
    return 0;
  }
  try {
    return this->currentTrackStream->readBytes((uint8_t*)ptr, nmemb * size);
  } catch (const std::exception& e) {
    CSPOT_LOG(error, "HTTP read error: %s", e.what());
    return 0;
  }
}

size_t TrackPlayer::_vorbisClose() {
  return 0;
}

int TrackPlayer::_vorbisSeek(int64_t offset, int whence) {
  if (this->currentTrackStream == nullptr) {
    return -1;
  }
  switch (whence) {
    case 0:
      this->currentTrackStream->seek(offset);
      break;
    case 1:
      this->currentTrackStream->seek(this->currentTrackStream->getPosition() +
                                     offset);
      break;
    case 2:
      this->currentTrackStream->seek(this->currentTrackStream->getSize() +
                                     offset);
      break;
  }

  return 0;
}

long TrackPlayer::_vorbisTell() {
  if (this->currentTrackStream == nullptr) {
    return 0;
  }
  return this->currentTrackStream->getPosition();
}

void TrackPlayer::setDataCallback(DataCallback callback) {
  this->dataCallback = callback;
}
