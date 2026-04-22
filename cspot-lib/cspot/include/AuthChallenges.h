#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Crypto.h"
#include "protobuf/authentication.pb.h"
#include "protobuf/keyexchange.pb.h"

namespace cspot {
class AuthChallenges {
 public:
  AuthChallenges();
  ~AuthChallenges();

  std::vector<uint8_t> prepareAuthPacket(std::vector<uint8_t>& authBlob,
                                         int authType,
                                         const std::string& deviceId,
                                         const std::string& username);

  std::vector<uint8_t> solveApHello(std::vector<uint8_t>& helloPacket,
                                    std::vector<uint8_t>& data);

  std::vector<uint8_t> prepareClientHello();

  std::vector<uint8_t> shanSendKey = {};
  std::vector<uint8_t> shanRecvKey = {};

 private:
  const long long SPOTIFY_VERSION = 0x10800000000;

  ClientResponseEncrypted authRequest;
  ClientResponsePlaintext clientResPlaintext;
  ClientHello clientHello;
  APResponseMessage apResponse;

  std::unique_ptr<Crypto> crypto;
};
}
