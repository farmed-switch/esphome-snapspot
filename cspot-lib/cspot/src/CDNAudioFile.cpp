#include "CDNAudioFile.h"

#include <string.h>
#include <exception>
#include <functional>
#include <initializer_list>
#include <map>
#include <string_view>
#include <type_traits>

#include "AccessKeyFetcher.h"
#include "BellLogger.h"
#include "Crypto.h"
#include "Logger.h"
#include "Packet.h"
#include "SocketStream.h"
#include "Utils.h"
#include "WrappedSemaphore.h"
#ifdef BELL_ONLY_CJSON
#include "cJSON.h"
#else
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#endif

using namespace cspot;

CDNAudioFile::CDNAudioFile(const std::string& cdnUrl,
                           const std::vector<uint8_t>& audioKey)
    : cdnUrl(cdnUrl), audioKey(audioKey) {
  this->crypto = std::make_unique<Crypto>();
}

size_t CDNAudioFile::getPosition() {
  return this->position;
}

void CDNAudioFile::seek(size_t newPos) {
  this->enableRequestMargin = true;
  this->position = newPos;

  this->lastRequestPosition = 0;
  this->lastRequestCapacity = 0;
  this->httpResponseExpected = 0;
  this->httpResponseRead = 0;
}

void CDNAudioFile::openStream() {
  CSPOT_LOG(info, "Opening HTTP stream to %s", this->cdnUrl.c_str());

  this->httpConnection = bell::HTTPClient::get(
      this->cdnUrl,
      {bell::HTTPClient::RangeHeader::range(0, OPUS_HEADER_SIZE - 1)});

  if (!this->httpConnection) {
    throw std::runtime_error("CDN: failed to open HTTP connection");
  }

  this->httpConnection->stream().read((char*)header.data(), OPUS_HEADER_SIZE);

  auto totalLength = this->httpConnection->totalLength();
  if (totalLength <= SPOTIFY_OPUS_HEADER) {
    throw std::runtime_error("CDN: file too short (totalLength=" +
                             std::to_string(totalLength) + ")");
  }
  this->totalFileSize = totalLength - SPOTIFY_OPUS_HEADER;

  this->decrypt(header.data(), OPUS_HEADER_SIZE, 0);

  size_t footerStartLocation =
      (this->totalFileSize - OPUS_FOOTER_PREFFERED + SPOTIFY_OPUS_HEADER) -
      (this->totalFileSize - OPUS_FOOTER_PREFFERED + SPOTIFY_OPUS_HEADER) % 16;

  this->footer = std::vector<uint8_t>(
      this->totalFileSize - footerStartLocation + SPOTIFY_OPUS_HEADER);
  this->httpConnection->get(
      cdnUrl, {bell::HTTPClient::RangeHeader::last(footer.size())});

  this->httpConnection->stream().read((char*)footer.data(),
                                      this->footer.size());

  this->decrypt(footer.data(), footer.size(), footerStartLocation);
  CSPOT_LOG(info, "Header and footer bytes received");
  this->position = 0;
  this->lastRequestPosition = 0;
  this->lastRequestCapacity = 0;
  this->httpResponseExpected = 0;
  this->httpResponseRead = 0;
}

size_t CDNAudioFile::readBytes(uint8_t* dst, size_t bytes) {
  size_t offsetPosition = position + SPOTIFY_OPUS_HEADER;
  size_t actualFileSize = this->totalFileSize + SPOTIFY_OPUS_HEADER;

  if (position >= this->totalFileSize) {
    return 0;
  }
  if (position + bytes > this->totalFileSize) {
    bytes = this->totalFileSize - position;
  }

  if (offsetPosition < OPUS_HEADER_SIZE &&
      bytes + offsetPosition <= OPUS_HEADER_SIZE) {
    memcpy(dst, this->header.data() + offsetPosition, bytes);
    position += bytes;
    return bytes;
  }

  if (offsetPosition >= (actualFileSize - this->footer.size())) {
    size_t toReadBytes = bytes;

    if ((position + bytes) > this->totalFileSize) {

      toReadBytes = this->totalFileSize - position;
    }

    size_t footerOffset =
        offsetPosition - (actualFileSize - this->footer.size());
    memcpy(dst, this->footer.data() + footerOffset, toReadBytes);

    position += toReadBytes;
    return toReadBytes;
  }

  if (offsetPosition >= this->lastRequestPosition &&
      offsetPosition < this->lastRequestPosition + this->lastRequestCapacity) {

    size_t toRead = bytes;
    if ((toRead + offsetPosition) >
        this->lastRequestPosition + lastRequestCapacity) {
      toRead = this->lastRequestPosition + lastRequestCapacity - offsetPosition;
    }
    memcpy(dst, this->httpBuffer.data() + offsetPosition - lastRequestPosition,
           toRead);
    position += toRead;
    return toRead;
  }

  if (offsetPosition >= this->lastRequestPosition + this->lastRequestCapacity &&
      this->httpResponseRead < this->httpResponseExpected &&
      offsetPosition < this->lastRequestPosition + this->httpResponseExpected) {

    size_t remaining = this->httpResponseExpected - this->httpResponseRead;
    size_t chunkSize = remaining < (size_t)STREAM_CHUNK_SIZE ? remaining : (size_t)STREAM_CHUNK_SIZE;

    try {
      this->httpConnection->stream().read(
          (char*)this->httpBuffer.data() + this->httpResponseRead, chunkSize);
    } catch (const std::exception& e) {
      CSPOT_LOG(error, "CDN stream read failed: %s", e.what());
      return 0;
    } catch (...) {
      CSPOT_LOG(error, "CDN stream read failed (unknown)");
      return 0;
    }
    size_t got = (size_t)this->httpConnection->stream().gcount();

    if (got > 0) {

      this->decrypt(this->httpBuffer.data() + this->httpResponseRead, got,
                    this->lastRequestPosition + this->httpResponseRead);
      this->httpResponseRead += got;
      this->lastRequestCapacity = this->httpResponseRead;
    }

    if (offsetPosition < this->lastRequestPosition + this->lastRequestCapacity) {
      size_t toRead = bytes;
      if ((toRead + offsetPosition) >
          this->lastRequestPosition + lastRequestCapacity) {
        toRead = this->lastRequestPosition + lastRequestCapacity - offsetPosition;
      }
      memcpy(dst, this->httpBuffer.data() + offsetPosition - lastRequestPosition,
             toRead);
      position += toRead;
      return toRead;
    }
  }

  {
    size_t requestPosition = (offsetPosition) - ((offsetPosition) % 16);
    if (this->enableRequestMargin && requestPosition > SEEK_MARGIN_SIZE) {
      requestPosition = (offsetPosition - SEEK_MARGIN_SIZE) -
                        ((offsetPosition - SEEK_MARGIN_SIZE) % 16);
      this->enableRequestMargin = false;
    }

    auto rangeHdr = bell::HTTPClient::RangeHeader::range(
        requestPosition, requestPosition + HTTP_BUFFER_SIZE - 1);

    if (this->httpConnection) {
      this->httpConnection->get(cdnUrl, {rangeHdr});
    } else {
      this->httpConnection = bell::HTTPClient::get(cdnUrl, {rangeHdr});
    }
    this->lastRequestPosition = requestPosition;

    int sc = this->httpConnection->statusCode();
    size_t expected = this->httpConnection->contentLength();
    CSPOT_LOG(info, "CDN range HTTP %d, contentLength=%zu, pos=%zu", sc, expected, requestPosition);

    if (sc != 206 && sc != 200) {
      throw std::runtime_error("CDN range request returned HTTP " + std::to_string(sc));
    }
    if (expected == 0) {
      expected = HTTP_BUFFER_SIZE;
    }

    this->httpResponseExpected = expected;
    this->httpResponseRead = 0;
    this->lastRequestCapacity = 0;

    size_t firstChunk = expected < (size_t)STREAM_CHUNK_SIZE ? expected : (size_t)STREAM_CHUNK_SIZE;
    this->httpConnection->stream().read((char*)this->httpBuffer.data(), firstChunk);
    size_t got = (size_t)this->httpConnection->stream().gcount();

    if (got == 0) {

      CSPOT_LOG(info, "CDN keep-alive stale at pos=%zu, fresh connection...",
                requestPosition);
      this->httpConnection = bell::HTTPClient::get(cdnUrl, {rangeHdr});
      sc = this->httpConnection->statusCode();
      expected = this->httpConnection->contentLength();
      if (sc != 206 && sc != 200) {
        throw std::runtime_error("CDN range request returned HTTP " + std::to_string(sc));
      }
      if (expected == 0) expected = HTTP_BUFFER_SIZE;
      this->httpResponseExpected = expected;
      firstChunk = expected < (size_t)STREAM_CHUNK_SIZE ? expected : (size_t)STREAM_CHUNK_SIZE;
      this->httpConnection->stream().read((char*)this->httpBuffer.data(), firstChunk);
      got = (size_t)this->httpConnection->stream().gcount();
      if (got == 0) {
        throw std::runtime_error(
            "CDN range read returned 0 bytes after reconnect (pos=" +
            std::to_string(requestPosition) + ")");
      }
    }

    this->decrypt(this->httpBuffer.data(), got, requestPosition);
    this->httpResponseRead = got;
    this->lastRequestCapacity = got;

    return readBytes(dst, bytes);
  }

  return bytes;
}

size_t CDNAudioFile::getSize() {
  return this->totalFileSize;
}

void CDNAudioFile::decrypt(uint8_t* dst, size_t nbytes, size_t pos) {
  auto calculatedIV = bigNumAdd(audioAESIV, pos / 16);

  this->crypto->aesCTRXcrypt(this->audioKey, calculatedIV, dst, nbytes);
}
