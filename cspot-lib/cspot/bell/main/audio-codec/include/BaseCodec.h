#pragma once

#include <stdint.h>

namespace bell {
class AudioContainer;

class BaseCodec {
 private:
  uint32_t lastSampleLen, availableBytes;

 public:
  uint32_t sampleRate = 44100;
  uint8_t channelCount = 2;
  uint8_t bitDepth = 16;

  virtual bool setup(AudioContainer* container);

  virtual bool setup(uint32_t sampleRate, uint8_t channelCount,
                     uint8_t bitDepth) = 0;

  virtual uint8_t* decode(uint8_t* inData, uint32_t& inLen,
                          uint32_t& outLen) = 0;

  uint8_t* decode(AudioContainer* container, uint32_t& outLen);

  int lastErrno = -1;
};
}
