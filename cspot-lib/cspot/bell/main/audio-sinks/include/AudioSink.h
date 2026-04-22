#ifndef AUDIOSINK_H
#define AUDIOSINK_H

#include <cstdint>
#include <cstdlib>
#include <vector>

class AudioSink {
 public:
  AudioSink() {}
  virtual ~AudioSink() {}
  virtual void feedPCMFrames(const uint8_t* buffer, size_t bytes) = 0;
  virtual void volumeChanged(uint16_t volume) {}

  virtual bool setParams(uint32_t sampleRate, uint8_t channelCount,
                         uint8_t bitDepth) {
    return false;
  }

  virtual inline bool setRate(uint16_t sampleRate) {
    return setParams(sampleRate, 2, 16);
  }
  bool softwareVolumeControl = true;
  bool usign = false;
};

#endif