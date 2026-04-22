#include "MercurySession.h"

#include <string.h>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <type_traits>
#include <utility>
#ifndef _WIN32
#include <arpa/inet.h>
#endif
#include "BellLogger.h"
#include "BellTask.h"
#include "BellUtils.h"
#include "Logger.h"
#include "NanoPBHelper.h"
#include "PlainConnection.h"
#include "ShannonConnection.h"
#include "TimeProvider.h"
#include "Utils.h"

using namespace cspot;

MercurySession::MercurySession(std::shared_ptr<TimeProvider> timeProvider)
    : bell::Task("mercury_dispatcher", 4 * 1024, 3, 1) {
  this->timeProvider = timeProvider;
}

MercurySession::~MercurySession() {
  std::scoped_lock lock(this->isRunningMutex);
}

void MercurySession::runTask() {
  isRunning = true;
  std::scoped_lock lock(this->isRunningMutex);

  this->executeEstabilishedCallback = true;
  while (isRunning) {

    if (!shanConn) {
      CSPOT_LOG(error, "shanConn is null, retrying reconnect...");
      reconnect();
      continue;
    }
    cspot::Packet packet = {};
    try {
      packet = shanConn->recvPacket();
      CSPOT_LOG(info, "Received packet, command: %d", packet.command);

      if (static_cast<RequestType>(packet.command) == RequestType::PING) {
        timeProvider->syncWithPingPacket(packet.data);

        this->lastPingTimestamp = timeProvider->getSyncedTimestamp();
        this->shanConn->sendPacket(0x49, packet.data);
      } else {
        this->packetQueue.push(packet);
      }
    } catch (const std::runtime_error& e) {
      CSPOT_LOG(error, "Error while receiving packet: %s", e.what());
      failAllPending();

      if (!isRunning)
        return;

      reconnect();
      continue;
    }
  }
}

void MercurySession::reconnect() {
  isReconnecting = true;

  try {
    this->conn = nullptr;
    this->shanConn = nullptr;

    this->connectWithRandomAp();
    this->authenticate(this->authBlob);

    CSPOT_LOG(info, "Reconnection successful");

    BELL_SLEEP_MS(100);

    lastPingTimestamp = timeProvider->getSyncedTimestamp();
    isReconnecting = false;

    this->executeEstabilishedCallback = true;
  } catch (...) {
    CSPOT_LOG(error, "Cannot reconnect, will retry in 5s");
    BELL_SLEEP_MS(5000);

  }
}

void MercurySession::setConnectedHandler(
    ConnectionEstabilishedCallback callback) {
  this->connectionReadyCallback = callback;
}

bool MercurySession::triggerTimeout() {
  if (!isRunning)
    return true;
  auto currentTimestamp = timeProvider->getSyncedTimestamp();

  if (currentTimestamp - this->lastPingTimestamp > PING_TIMEOUT_MS) {
    CSPOT_LOG(debug, "Reconnection required, no ping received");
    return true;
  }

  return false;
}

void MercurySession::unregister(uint64_t sequenceId) {
  auto callback = this->callbacks.find(sequenceId);

  if (callback != this->callbacks.end()) {
    this->callbacks.erase(callback);
  }
}

void MercurySession::unregisterAudioKey(uint32_t sequenceId) {
  auto callback = this->audioKeyCallbacks.find(sequenceId);

  if (callback != this->audioKeyCallbacks.end()) {
    this->audioKeyCallbacks.erase(callback);
  }
}

void MercurySession::disconnect() {
  CSPOT_LOG(info, "Disconnecting mercury session");

  this->isRunning = false;

  if (conn) {
    conn->close();
  }

  BELL_SLEEP_MS(50);

  std::scoped_lock lock(this->isRunningMutex);

}

void MercurySession::closeConnection() {
  CSPOT_LOG(info, "Closing connection (non-blocking)");
  this->isRunning = false;
  if (conn) {
    conn->close();
  }
}

std::string MercurySession::getCountryCode() {
  return this->countryCode;
}

void MercurySession::handlePacket() {
  Packet packet = {};

  if (!this->packetQueue.wtpop(packet, 200)) {

    if (executeEstabilishedCallback && this->connectionReadyCallback != nullptr) {
      executeEstabilishedCallback = false;
      this->connectionReadyCallback();
    }
    return;
  }

  if (executeEstabilishedCallback && this->connectionReadyCallback != nullptr) {
    executeEstabilishedCallback = false;
    this->connectionReadyCallback();
  }

  switch (static_cast<RequestType>(packet.command)) {
    case RequestType::COUNTRY_CODE_RESPONSE: {
      this->countryCode = std::string();
      this->countryCode.resize(2);
      memcpy(this->countryCode.data(), packet.data.data(), 2);
      CSPOT_LOG(debug, "Received country code %s", this->countryCode.c_str());
      break;
    }
    case RequestType::AUDIO_KEY_FAILURE_RESPONSE:
    case RequestType::AUDIO_KEY_SUCCESS_RESPONSE: {

      auto seqId = ntohl(extract<uint32_t>(packet.data, 0));

      if (this->audioKeyCallbacks.count(seqId) > 0) {
        auto success = static_cast<RequestType>(packet.command) ==
                       RequestType::AUDIO_KEY_SUCCESS_RESPONSE;
        this->audioKeyCallbacks[seqId](success, packet.data);
      }

      break;
    }
    case RequestType::SEND:
    case RequestType::SUB:
    case RequestType::UNSUB: {
      CSPOT_LOG(debug, "Received mercury packet");

      auto response = this->decodeResponse(packet.data);
      if (this->callbacks.count(response.sequenceId) > 0) {
        auto seqId = response.sequenceId;
        this->callbacks[response.sequenceId](response);
        this->callbacks.erase(this->callbacks.find(seqId));
      }
      break;
    }
    case RequestType::SUBRES: {
      auto response = decodeResponse(packet.data);

      auto uri = std::string(response.mercuryHeader.uri);
      if (this->subscriptions.count(uri) > 0) {
        this->subscriptions[uri](response);
      }
      break;
    }
    default:
      break;
  }
}

void MercurySession::failAllPending() {
  Response response = {};
  response.fail = true;

  for (auto& it : this->callbacks) {
    it.second(response);
  }

  for (auto& it : this->subscriptions) {
    it.second(response);
  }

  this->subscriptions = {};
  this->callbacks = {};
}

MercurySession::Response MercurySession::decodeResponse(
    const std::vector<uint8_t>& data) {
  Response response = {};
  response.parts = {};

  if (data.size() < 15) {
    CSPOT_LOG(error, "decodeResponse: data too small (%zu bytes)", data.size());
    response.fail = true;
    return response;
  }

  response.sequenceId = hton64(extract<uint64_t>(data, 2));

  auto headerSize = ntohs(extract<uint16_t>(data, 13));
  if (data.size() < (size_t)(15 + headerSize)) {
    CSPOT_LOG(error, "decodeResponse: data smaller than declared header (%zu < %zu)",
              data.size(), (size_t)(15 + headerSize));
    response.fail = true;
    return response;
  }

  auto headerBytes =
      std::vector<uint8_t>(data.begin() + 15, data.begin() + 15 + headerSize);

  auto pos = 15 + headerSize;
  while (pos + 2 <= data.size()) {
    auto partSize = ntohs(extract<uint16_t>(data, pos));
    if (pos + 2 + partSize > data.size()) {
      CSPOT_LOG(error, "decodeResponse: part overflows data (pos=%zu partSize=%u dataSize=%zu)",
                pos, partSize, data.size());
      break;
    }
    response.parts.push_back(std::vector<uint8_t>(
        data.begin() + pos + 2, data.begin() + pos + 2 + partSize));
    pos += 2 + partSize;
  }

  pbDecode(response.mercuryHeader, Header_fields, headerBytes);
  response.fail = false;

  return response;
}

uint64_t MercurySession::executeSubscription(RequestType method,
                                             const std::string& uri,
                                             ResponseCallback callback,
                                             ResponseCallback subscription,
                                             DataParts& payload) {
  CSPOT_LOG(debug, "Executing Mercury Request, type %s",
            RequestTypeMap[method].c_str());

  pbPutString(uri, tempMercuryHeader.uri);
  pbPutString(RequestTypeMap[method], tempMercuryHeader.method);

  tempMercuryHeader.has_method = true;
  tempMercuryHeader.has_uri = true;

  if (method == RequestType::GET) {
    method = RequestType::SEND;
  }

  if (method == RequestType::SUB) {
    this->subscriptions.insert({uri, subscription});
  }

  auto headerBytes = pbEncode(Header_fields, &tempMercuryHeader);

  this->callbacks.insert({sequenceId, callback});

  auto sequenceIdBytes = pack<uint64_t>(hton64(this->sequenceId));
  auto sequenceSizeBytes = pack<uint16_t>(htons(sequenceIdBytes.size()));

  sequenceIdBytes.insert(sequenceIdBytes.begin(), sequenceSizeBytes.begin(),
                         sequenceSizeBytes.end());
  sequenceIdBytes.push_back(0x01);

  auto payloadNum = pack<uint16_t>(htons(payload.size() + 1));
  sequenceIdBytes.insert(sequenceIdBytes.end(), payloadNum.begin(),
                         payloadNum.end());

  auto headerSizePayload = pack<uint16_t>(htons(headerBytes.size()));
  sequenceIdBytes.insert(sequenceIdBytes.end(), headerSizePayload.begin(),
                         headerSizePayload.end());
  sequenceIdBytes.insert(sequenceIdBytes.end(), headerBytes.begin(),
                         headerBytes.end());

  for (int x = 0; x < payload.size(); x++) {
    headerSizePayload = pack<uint16_t>(htons(payload[x].size()));
    sequenceIdBytes.insert(sequenceIdBytes.end(), headerSizePayload.begin(),
                           headerSizePayload.end());
    sequenceIdBytes.insert(sequenceIdBytes.end(), payload[x].begin(),
                           payload[x].end());
  }

  this->sequenceId += 1;

  try {
    this->shanConn->sendPacket(
        static_cast<std::underlying_type<RequestType>::type>(method),
        sequenceIdBytes);
  } catch (...) {
    CSPOT_LOG(error, "sendPacket failed for Mercury request (seq %llu)",
              (unsigned long long)(this->sequenceId - 1));

    auto it = this->callbacks.find(this->sequenceId - 1);
    if (it != this->callbacks.end()) {
      Response fail;
      fail.fail = true;
      it->second(fail);
      this->callbacks.erase(it);
    }
  }

  return this->sequenceId - 1;
}

uint32_t MercurySession::requestAudioKey(const std::vector<uint8_t>& trackId,
                                         const std::vector<uint8_t>& fileId,
                                         AudioKeyCallback audioCallback) {
  auto buffer = fileId;

  this->audioKeyCallbacks.insert({this->audioKeySequence, audioCallback});

  buffer.insert(buffer.end(), trackId.begin(), trackId.end());
  auto audioKeySequenceBuffer = pack<uint32_t>(htonl(this->audioKeySequence));
  buffer.insert(buffer.end(), audioKeySequenceBuffer.begin(),
                audioKeySequenceBuffer.end());
  auto suffix = std::vector<uint8_t>({0x00, 0x00});
  buffer.insert(buffer.end(), suffix.begin(), suffix.end());

  this->audioKeySequence += 1;

  try {
    this->shanConn->sendPacket(
        static_cast<uint8_t>(RequestType::AUDIO_KEY_REQUEST_COMMAND), buffer);
  } catch (...) {
    CSPOT_LOG(error, "sendPacket failed for audio key request (seq %u)",
              this->audioKeySequence - 1);

    auto it = this->audioKeyCallbacks.find(this->audioKeySequence - 1);
    if (it != this->audioKeyCallbacks.end()) {
      it->second(false, {});
      this->audioKeyCallbacks.erase(it);
    }
  }
  return audioKeySequence - 1;
}
