#pragma once

#include <stddef.h>
#include <stdint.h>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

#include "BellTask.h"
#include "ByteStream.h"
#include "WrappedSemaphore.h"

class BufferedStream : public bell::ByteStream, bell::Task {
 public:
  typedef std::shared_ptr<bell::ByteStream> StreamPtr;
  typedef std::function<StreamPtr(uint32_t rangeStart)> StreamReader;

 public:

  BufferedStream(const std::string& taskName, uint32_t bufferSize,
                 uint32_t readThreshold, uint32_t readSize,
                 uint32_t readyThreshold, uint32_t notReadyThreshold,
                 bool waitForReady = false);
  ~BufferedStream() override;
  bool open(const StreamPtr& stream);
  bool open(const StreamReader& newReader, uint32_t initialOffset = 0);
  void close() override;

 public:

  size_t read(uint8_t* dst, size_t len) override;
  size_t skip(size_t len) override;
  size_t position() override;
  size_t size() override;

 public:

  uint32_t readTotal;

  uint32_t bufferTotal;

  std::atomic<uint32_t> readAvailable;

  bool isReady() const;

  bool isNotReady() const;

  bell::WrappedSemaphore readySem;

 private:
  std::mutex runningMutex;
  bool running = false;
  bool terminate = false;
  bell::WrappedSemaphore
      readSem;
  std::mutex
      readMutex;
  uint32_t bufferSize;
  uint32_t readAt;
  uint32_t readSize;
  uint32_t readyThreshold;
  uint32_t notReadyThreshold;
  bool waitForReady;
  uint8_t* buf;
  uint8_t* bufEnd;
  uint8_t* bufReadPtr;
  uint8_t* bufWritePtr;
  StreamPtr source;
  StreamReader reader;
  void runTask() override;
  void reset();
  uint32_t lengthBetween(uint8_t* me, uint8_t* other);
};
