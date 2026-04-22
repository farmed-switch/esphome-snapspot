#ifndef PLAINCONNECTION_H
#define PLAINCONNECTION_H
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

#include "win32shim.h"
#else
#include <unistd.h>
#endif
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

typedef std::function<bool()> timeoutCallback;

namespace cspot {
class PlainConnection {
 public:
  PlainConnection();
  ~PlainConnection();

  void connect(const std::string& apAddress);
  void close();

  timeoutCallback timeoutHandler;
  std::vector<uint8_t> sendPrefixPacket(const std::vector<uint8_t>& prefix,
                                        const std::vector<uint8_t>& data);
  std::vector<uint8_t> recvPacket();

  void readBlock(const uint8_t* dst, size_t size);
  size_t writeBlock(const std::vector<uint8_t>& data);

 private:
  int apSock;
};
}

#endif