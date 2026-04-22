#pragma once

#include <stdint.h>
#include <cstddef>
#include <istream>
#include <vector>

#include "AudioContainer.h"
#include "CodecType.h"

namespace bell {
class ADTSContainer : public AudioContainer {
 public:
  ~ADTSContainer(){};
  ADTSContainer(std::istream& istr, const std::byte* headingBytes = nullptr);

  std::byte* readSample(uint32_t& len) override;
  bool resyncADTS();
  void parseSetupData() override;
  void consumeBytes(uint32_t len) override;

  bell::AudioCodec getCodec() override { return bell::AudioCodec::AAC; }

 private:
  static constexpr auto AAC_MAX_FRAME_SIZE = 2100;
  static constexpr auto BUFFER_SIZE = 1024 * 10;

  std::vector<std::byte> buffer = std::vector<std::byte>(BUFFER_SIZE);

  size_t bytesInBuffer = 0;
  size_t dataOffset = 0;
  bool protectionAbsent = false;

  bool fillBuffer();
};
}
