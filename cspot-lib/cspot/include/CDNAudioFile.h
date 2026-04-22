#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Crypto.h"
#include "HTTPClient.h"

namespace bell {
class WrappedSemaphore;
}

namespace cspot {
class AccessKeyFetcher;

class CDNAudioFile {

 public:
  CDNAudioFile(const std::string& cdnUrl, const std::vector<uint8_t>& audioKey);

  void openStream();

  size_t readBytes(uint8_t* dst, size_t bytes);

  size_t getPosition();

  size_t getSize();

  void seek(size_t position);

 private:
  const int OPUS_HEADER_SIZE = 8 * 1024;
  const int OPUS_FOOTER_PREFFERED = 1024 * 12;
  const int SEEK_MARGIN_SIZE = 1024 * 4;

  const int HTTP_BUFFER_SIZE = 1024 * 14;
  const int STREAM_CHUNK_SIZE = 1024 * 14;
  const int SPOTIFY_OPUS_HEADER = 167;

  std::vector<uint8_t> header = std::vector<uint8_t>(OPUS_HEADER_SIZE);
  std::vector<uint8_t> footer;

  std::vector<uint8_t> httpBuffer = std::vector<uint8_t>(HTTP_BUFFER_SIZE);

  const std::vector<uint8_t> audioAESIV = {0x72, 0xe0, 0x67, 0xfb, 0xdd, 0xcb,
                                           0xcf, 0x77, 0xeb, 0xe8, 0xbc, 0x64,
                                           0x3f, 0x63, 0x0d, 0x93};
  std::unique_ptr<Crypto> crypto;

  std::unique_ptr<bell::HTTPClient::Response> httpConnection;

  size_t position = 0;
  size_t totalFileSize = 0;
  size_t lastRequestPosition = 0;
  size_t lastRequestCapacity = 0;
  size_t httpResponseExpected = 0;
  size_t httpResponseRead = 0;

  bool enableRequestMargin = false;

  std::string cdnUrl;
  std::vector<uint8_t> audioKey;

  void decrypt(uint8_t* dst, size_t nbytes, size_t pos);
};
}
