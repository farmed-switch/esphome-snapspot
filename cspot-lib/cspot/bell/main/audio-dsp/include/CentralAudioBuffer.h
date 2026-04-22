#pragma once

#include <atomic>
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>

#include "BellUtils.h"
#include "CircularBuffer.h"
#include "StreamInfo.h"
#include "WrappedSemaphore.h"

#ifdef _WIN32
#define __attribute__(X)
#endif

typedef std::function<void(std::string)> shutdownEventHandler;

namespace bell {
class CentralAudioBuffer {
 private:
  std::mutex accessMutex;

  std::atomic<bool> isLocked = false;
  std::mutex dataAccessMutex;

 public:
  static const size_t PCM_CHUNK_SIZE = 4096;
  std::unique_ptr<bell::WrappedSemaphore> chunkReady;

  struct AudioChunk {

    int32_t sec;
    int32_t usec;

    size_t trackHash;

    uint32_t sampleRate;
    uint8_t channels;
    uint8_t bitWidth;

    size_t pcmSize;

    uint8_t pcmData[PCM_CHUNK_SIZE];
  } __attribute__((packed));

  CentralAudioBuffer(size_t chunks) {
    audioBuffer = std::make_shared<CircularBuffer>(chunks * sizeof(AudioChunk));
    chunkReady = std::make_unique<bell::WrappedSemaphore>(50);
  }

  std::shared_ptr<bell::CircularBuffer> audioBuffer;
  uint32_t currentSampleRate = 44100;

  uint32_t getSampleRate() { return currentSampleRate; }

  void clearBuffer() {
    std::scoped_lock lock(this->dataAccessMutex);

    audioBuffer->emptyBuffer();
    hasChunk = false;
  }

  void emptyCompletely() {
    std::scoped_lock lock(this->dataAccessMutex);
    audioBuffer->emptyBuffer();
  }

  bool hasAtLeast(size_t chunks) {
    return this->audioBuffer->size() >= chunks * sizeof(AudioChunk);
  }

  void lockAccess() {
    if (!isLocked) {
      clearBuffer();
      this->accessMutex.lock();
      isLocked = true;
    }
  }

  void unlockAccess() {
    if (isLocked) {
      clearBuffer();
      this->accessMutex.unlock();
      isLocked = false;
    }
  }

  AudioChunk currentChunk = {};
  bool hasChunk = false;

  AudioChunk lastReadChunk = {};

  AudioChunk* readChunk() {
    std::scoped_lock lock(this->dataAccessMutex);
    if (audioBuffer->size() < sizeof(AudioChunk)) {
      lastReadChunk.pcmSize = 0;
      return nullptr;
    }

    audioBuffer->read((uint8_t*)&lastReadChunk, sizeof(AudioChunk));
    currentSampleRate = static_cast<uint32_t>(lastReadChunk.sampleRate);
    return &lastReadChunk;
  }

  size_t writePCM(const uint8_t* data, size_t dataSize, size_t hash,
                  uint32_t sampleRate = 44100, uint8_t channels = 2,
                  BitWidth bitWidth = BitWidth::BW_16, int32_t sec = 0,
                  int32_t usec = 0) {
    std::scoped_lock lock(this->dataAccessMutex);
    if (hasChunk && (currentChunk.trackHash != hash ||
                     currentChunk.pcmSize >= PCM_CHUNK_SIZE)) {

      if ((audioBuffer->size() - audioBuffer->capacity()) <
          sizeof(AudioChunk)) {
        return 0;
      }

      hasChunk = false;
      this->audioBuffer->write((uint8_t*)&currentChunk, sizeof(AudioChunk));

    }

    if (!hasChunk) {
      currentChunk.trackHash = hash;
      currentChunk.sampleRate = sampleRate;
      currentChunk.channels = channels;
      currentChunk.bitWidth = 16;
      currentChunk.sec = sec;
      currentChunk.usec = usec;
      currentChunk.pcmSize = 0;
      hasChunk = true;
    }

    size_t toWriteSize = dataSize;

    if (currentChunk.pcmSize + toWriteSize > PCM_CHUNK_SIZE) {
      toWriteSize = PCM_CHUNK_SIZE - currentChunk.pcmSize;
    }

    memcpy(currentChunk.pcmData + currentChunk.pcmSize, data, toWriteSize);
    currentChunk.pcmSize += toWriteSize;

    return toWriteSize;
  }
};

}
