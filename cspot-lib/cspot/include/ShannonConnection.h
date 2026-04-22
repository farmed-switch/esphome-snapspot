#ifndef SHANNONCONNECTION_H
#define SHANNONCONNECTION_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

#include "Packet.h"

class Shannon;
namespace cspot {

class PlainConnection;
}

#define MAC_SIZE 4
namespace cspot {
class ShannonConnection {
 private:
  std::unique_ptr<Shannon> sendCipher;
  std::unique_ptr<Shannon> recvCipher;
  uint32_t sendNonce = 0;
  uint32_t recvNonce = 0;
  std::vector<uint8_t> cipherPacket(uint8_t cmd, std::vector<uint8_t>& data);
  std::mutex writeMutex;
  std::mutex readMutex;

 public:
  ShannonConnection();
  ~ShannonConnection();
  void wrapConnection(std::shared_ptr<PlainConnection> conn,
                      std::vector<uint8_t>& sendKey,
                      std::vector<uint8_t>& recvKey);
  void sendPacket(uint8_t cmd, std::vector<uint8_t>& data);
  std::shared_ptr<PlainConnection> conn;
  Packet recvPacket();
};
}

#endif
