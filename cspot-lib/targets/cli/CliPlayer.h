#pragma once

#include <stddef.h>
#include <stdint.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>

#include "AudioSink.h"
#include "BellTask.h"

namespace bell {
class BellDSP;
class CentralAudioBuffer;
}
namespace cspot {
class SpircHandler;
}

class CliPlayer : public bell::Task {
 public:
  CliPlayer(std::unique_ptr<AudioSink> sink,
            std::shared_ptr<cspot::SpircHandler> spircHandler);
  void disconnect();

 private:
  std::string currentTrackId;
  std::shared_ptr<cspot::SpircHandler> handler;
  std::shared_ptr<bell::BellDSP> dsp;
  std::unique_ptr<AudioSink> audioSink;
  std::shared_ptr<bell::CentralAudioBuffer> centralAudioBuffer;

  void feedData(uint8_t* data, size_t len);

  std::atomic<bool> pauseRequested = false;
  std::atomic<bool> isPaused = true;
  std::atomic<bool> isRunning = true;
  std::mutex runningMutex;
  std::atomic<bool> playlistEnd = false;

  void runTask() override;
};
