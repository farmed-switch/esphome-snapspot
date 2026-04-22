#pragma once

#include <atomic>
#include <cstdint>
#include <ctime>
#include <functional>
#include <memory>
#include <mutex>
#include <string_view>
#include <vector>

#include "BellTask.h"
#include "CDNAudioFile.h"
#include "TrackQueue.h"

namespace bell {
class WrappedSemaphore;
}

#ifdef BELL_VORBIS_FLOAT
#include "vorbis/vorbisfile.h"
#else
#include "ivorbisfile.h"
#endif

namespace cspot {
class TrackProvider;
class TrackQueue;
struct Context;
struct TrackReference;

class TrackPlayer : bell::Task {
 public:

  typedef std::function<void(std::shared_ptr<QueuedTrack>, bool)>
      TrackLoadedCallback;
  typedef std::function<size_t(uint8_t*, size_t, std::string_view)>
      DataCallback;
  typedef std::function<void()> EOFCallback;

  TrackPlayer(std::shared_ptr<cspot::Context> ctx,
              std::shared_ptr<cspot::TrackQueue> trackQueue,
              EOFCallback eofCallback, TrackLoadedCallback loadedCallback);
  ~TrackPlayer();

  void loadTrackFromRef(TrackReference& ref, size_t playbackMs,
                        bool startAutomatically);
  void setDataCallback(DataCallback callback);

  void seekMs(size_t ms);
  void resetState(bool paused = false);

  size_t _vorbisRead(void* ptr, size_t size, size_t nmemb);
  size_t _vorbisClose();
  int _vorbisSeek(int64_t offset, int whence);
  long _vorbisTell();

  void stop();
  void start();

 private:
  std::shared_ptr<cspot::Context> ctx;
  std::shared_ptr<cspot::TrackQueue> trackQueue;
  std::shared_ptr<cspot::CDNAudioFile> currentTrackStream;

  std::unique_ptr<bell::WrappedSemaphore> playbackSemaphore;

  TrackLoadedCallback trackLoaded;
  DataCallback dataCallback = nullptr;
  EOFCallback eofCallback;

  std::atomic<bool> currentSongPlaying;
  std::mutex playbackMutex;
  std::mutex dataOutMutex;

  OggVorbis_File vorbisFile;
  ov_callbacks vorbisCallbacks;
  int currentSection;

  std::vector<uint8_t> pcmBuffer = std::vector<uint8_t>(1024);

  bool autoStart = false;

  std::atomic<bool> isRunning = false;
  std::atomic<bool> pendingReset = false;
  std::atomic<bool> inFuture = false;
  std::atomic<size_t> pendingSeekPositionMs = 0;
  std::atomic<bool> startPaused = false;
  std::atomic<bool> taskStarted_{false};

  std::unique_ptr<bell::WrappedSemaphore> taskDoneSem_;

  void runTask() override;
};
}
