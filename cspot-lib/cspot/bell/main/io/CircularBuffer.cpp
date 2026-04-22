#include "CircularBuffer.h"

#include <algorithm>
#include <stdexcept>

#if defined(ESP_PLATFORM)
#include "esp_heap_caps.h"
#define CIRBUF_MALLOC(sz)  heap_caps_malloc((sz), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
#define CIRBUF_FREE(p)     heap_caps_free(p)
#else
#define CIRBUF_MALLOC(sz)  malloc(sz)
#define CIRBUF_FREE(p)     free(p)
#endif

using namespace bell;

CircularBuffer::CircularBuffer(size_t dataCapacity) {
  this->dataCapacity = dataCapacity;
  buffer = static_cast<uint8_t*>(CIRBUF_MALLOC(dataCapacity));
  if (!buffer) {

    buffer = static_cast<uint8_t*>(malloc(dataCapacity));
  }
  if (!buffer) {
    throw std::runtime_error("CircularBuffer: failed to allocate buffer");
  }
  this->dataSemaphore = std::make_unique<bell::WrappedSemaphore>(5);
};

CircularBuffer::~CircularBuffer() {
  if (buffer) {
    CIRBUF_FREE(buffer);
    buffer = nullptr;
  }
}

size_t CircularBuffer::write(const uint8_t* data, size_t bytes) {
  if (bytes == 0)
    return 0;

  std::lock_guard<std::mutex> guard(bufferMutex);
  size_t bytesToWrite = std::min(bytes, dataCapacity - dataSize);

  if (bytesToWrite <= dataCapacity - endIndex) {
    memcpy(buffer + endIndex, data, bytesToWrite);
    endIndex += bytesToWrite;
    if (endIndex == dataCapacity)
      endIndex = 0;
  }

  else {
    size_t firstChunkSize = dataCapacity - endIndex;
    memcpy(buffer + endIndex, data, firstChunkSize);
    size_t secondChunkSize = bytesToWrite - firstChunkSize;
    memcpy(buffer, data + firstChunkSize, secondChunkSize);
    endIndex = secondChunkSize;
  }

  dataSize += bytesToWrite;

  return bytesToWrite;
}

void CircularBuffer::emptyBuffer() {
  std::lock_guard<std::mutex> guard(bufferMutex);
  begIndex = 0;
  dataSize = 0;
  endIndex = 0;
}

void CircularBuffer::emptyExcept(size_t sizeToSet) {
  std::lock_guard<std::mutex> guard(bufferMutex);
  if (sizeToSet > dataSize)
    sizeToSet = dataSize;
  dataSize = sizeToSet;
  endIndex = begIndex + sizeToSet;
  if (endIndex > dataCapacity) {
    endIndex -= dataCapacity;
  }
}

size_t CircularBuffer::read(uint8_t* data, size_t bytes) {
  if (bytes == 0)
    return 0;

  std::lock_guard<std::mutex> guard(bufferMutex);
  size_t bytesToRead = std::min(bytes, dataSize);

  if (bytesToRead <= dataCapacity - begIndex) {
    memcpy(data, buffer + begIndex, bytesToRead);
    begIndex += bytesToRead;
    if (begIndex == dataCapacity)
      begIndex = 0;
  }

  else {
    size_t firstChunkSize = dataCapacity - begIndex;
    memcpy(data, buffer + begIndex, firstChunkSize);
    size_t secondChunkSize = bytesToRead - firstChunkSize;
    memcpy(data + firstChunkSize, buffer, secondChunkSize);
    begIndex = secondChunkSize;
  }

  dataSize -= bytesToRead;
  return bytesToRead;
}
