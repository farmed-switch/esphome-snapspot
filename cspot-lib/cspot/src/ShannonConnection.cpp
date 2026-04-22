#include "ShannonConnection.h"

#include <type_traits>

#include "BellLogger.h"
#include "Logger.h"
#include "Packet.h"
#include "PlainConnection.h"
#include "Shannon.h"
#include "Utils.h"
#ifndef _WIN32
#include <arpa/inet.h>
#endif

using namespace cspot;

ShannonConnection::ShannonConnection() {}

ShannonConnection::~ShannonConnection() {}

void ShannonConnection::wrapConnection(
    std::shared_ptr<cspot::PlainConnection> conn, std::vector<uint8_t>& sendKey,
    std::vector<uint8_t>& recvKey) {
  this->conn = conn;

  this->sendCipher = std::make_unique<Shannon>();
  this->recvCipher = std::make_unique<Shannon>();

  this->sendCipher->key(sendKey);
  this->recvCipher->key(recvKey);

  this->sendCipher->nonce(pack<uint32_t>(htonl(0)));
  this->recvCipher->nonce(pack<uint32_t>(htonl(0)));
}

void ShannonConnection::sendPacket(uint8_t cmd, std::vector<uint8_t>& data) {
  std::scoped_lock lock(this->writeMutex);
  auto rawPacket = this->cipherPacket(cmd, data);

  this->sendCipher->encrypt(rawPacket);
  this->conn->writeBlock(rawPacket);

  std::vector<uint8_t> mac(MAC_SIZE);
  this->sendCipher->finish(mac);

  this->sendNonce += 1;
  this->sendCipher->nonce(pack<uint32_t>(htonl(this->sendNonce)));

  this->conn->writeBlock(mac);
}

cspot::Packet ShannonConnection::recvPacket() {
  std::scoped_lock lock(this->readMutex);

  std::vector<uint8_t> data(3);

  this->conn->readBlock(data.data(), 3);
  this->recvCipher->decrypt(data);

  auto readSize = ntohs(extract<uint16_t>(data, 1));
  if (readSize > 8192) {
    CSPOT_LOG(error, "Shannon packet size %u exceeds limit", readSize);
    throw std::runtime_error("Shannon packet too large");
  }
  auto packetData = std::vector<uint8_t>(readSize);

  if (readSize > 0) {
    this->conn->readBlock(packetData.data(), readSize);
    this->recvCipher->decrypt(packetData);
  }

  std::vector<uint8_t> mac(MAC_SIZE);
  this->conn->readBlock(mac.data(), MAC_SIZE);

  std::vector<uint8_t> mac2(MAC_SIZE);
  this->recvCipher->finish(mac2);

  if (mac != mac2) {
    CSPOT_LOG(error, "Shannon MAC mismatch — connection out of sync");
    throw std::runtime_error("Shannon MAC verification failed");
  }

  this->recvNonce += 1;
  this->recvCipher->nonce(pack<uint32_t>(htonl(this->recvNonce)));
  uint8_t cmd = 0;
  if (data.size() > 0) {
    cmd = data[0];
  }

  return Packet{cmd, packetData};
}

std::vector<uint8_t> ShannonConnection::cipherPacket(
    uint8_t cmd, std::vector<uint8_t>& data) {

  auto sizeRaw = pack<uint16_t>(htons(uint16_t(data.size())));

  sizeRaw.insert(sizeRaw.begin(), cmd);
  sizeRaw.insert(sizeRaw.end(), data.begin(), data.end());

  return sizeRaw;
}
