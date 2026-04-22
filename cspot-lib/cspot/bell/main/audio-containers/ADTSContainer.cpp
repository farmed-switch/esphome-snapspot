#include "ADTSContainer.h"

#include <cstring>
#include <iostream>

#include "StreamInfo.h"

using namespace bell;

#define SYNC_WORLD_LEN 4
#define SYNCWORDH 0xff
#define SYNCWORDL 0xf0

#define AAC_ADTS_FRAME_HEADER_LEN 9

#define AAC_ADTS_SYNC_VERIFY(buf) \
  ((buf[0] == 0xff) && ((buf[1] & 0xf6) == 0xf0))

#define AAC_ADTS_FRAME_GETSIZE(buf) \
  ((buf[3] & 0x03) << 11 | buf[4] << 3 | buf[5] >> 5)

ADTSContainer::ADTSContainer(std::istream& istr, const std::byte* headingBytes)
    : bell::AudioContainer(istr) {
  if (headingBytes != nullptr) {
    memcpy(buffer.data(), headingBytes, 7);
    bytesInBuffer = 7;
  }
}

bool ADTSContainer::fillBuffer() {
  if (this->bytesInBuffer < AAC_MAX_FRAME_SIZE * 2) {
    this->istr.read((char*)buffer.data() + bytesInBuffer,
                    buffer.size() - bytesInBuffer);
    this->bytesInBuffer += istr.gcount();
  }
  return this->bytesInBuffer >= AAC_MAX_FRAME_SIZE;
}

bool ADTSContainer::resyncADTS() {
  int resyncOffset = 0;
  bool resyncValid = false;

  size_t validBytes = bytesInBuffer - dataOffset;

  while (!resyncValid && resyncOffset < validBytes) {
    uint8_t* buf = (uint8_t*)this->buffer.data() + dataOffset + resyncOffset;
    if (AAC_ADTS_SYNC_VERIFY(buf)) {

      uint32_t frameSize = AAC_ADTS_FRAME_GETSIZE(buf);

      if (frameSize + resyncOffset > validBytes) {

        resyncOffset++;
        continue;
      }

      buf =
          (uint8_t*)this->buffer.data() + dataOffset + resyncOffset + frameSize;

      if (AAC_ADTS_SYNC_VERIFY(buf)) {
        buf += AAC_ADTS_FRAME_GETSIZE(buf);
        if (AAC_ADTS_SYNC_VERIFY(buf)) {
          protectionAbsent = (buf[1] & 1);

          resyncValid = true;
        }
      }
    } else {
      resyncOffset++;
    }
  }

  dataOffset += resyncOffset;
  return resyncValid;
}

void ADTSContainer::consumeBytes(uint32_t len) {
  dataOffset += len;
}

std::byte* ADTSContainer::readSample(uint32_t& len) {

  if (dataOffset > 0 && bytesInBuffer > 0) {
    size_t toConsume = std::min(dataOffset, bytesInBuffer);
    memmove(buffer.data(), buffer.data() + toConsume,
            buffer.size() - toConsume);

    dataOffset -= toConsume;
    bytesInBuffer -= toConsume;
  }

  if (!this->fillBuffer()) {
    len = 0;
    return nullptr;
  }

  uint8_t* buf = (uint8_t*)buffer.data() + dataOffset;

  if (!AAC_ADTS_SYNC_VERIFY(buf)) {
    if (!resyncADTS()) {
      len = 0;
      return nullptr;
    }
  } else {
    protectionAbsent = (buf[1] & 1);
  }

  len = AAC_ADTS_FRAME_GETSIZE(buf);

  if (len > bytesInBuffer - dataOffset) {
    if (!resyncADTS()) {
      len = 0;
      return nullptr;
    }
  }

  return buffer.data() + dataOffset;
}

void ADTSContainer::parseSetupData() {
  channels = 2;
  sampleRate = bell::SampleRate::SR_44100;
  bitWidth = bell::BitWidth::BW_16;
}
