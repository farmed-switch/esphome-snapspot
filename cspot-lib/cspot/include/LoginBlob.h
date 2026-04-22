#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Crypto.h"

namespace cspot {
class LoginBlob {
 private:
  int blobSkipPosition = 0;
  std::unique_ptr<Crypto> crypto;
  std::string name, deviceId;

  uint32_t readBlobInt(const std::vector<uint8_t>& loginData);
  std::vector<uint8_t> decodeBlob(const std::vector<uint8_t>& blob,
                                  const std::vector<uint8_t>& sharedKey);
  std::vector<uint8_t> decodeBlobSecondary(const std::vector<uint8_t>& blob,
                                           const std::string& username,
                                           const std::string& deviceId);

 public:
  LoginBlob(std::string name);
  std::vector<uint8_t> authData;
  std::string username = "";
  int authType;

  void loadZeroconfQuery(std::map<std::string, std::string>& queryParams);
  void loadZeroconf(const std::vector<uint8_t>& blob,
                    const std::vector<uint8_t>& sharedKey,
                    const std::string& deviceId, const std::string& username);
  void loadUserPass(const std::string& username, const std::string& password);
  void loadJson(const std::string& json);

  std::string buildZeroconfInfo();
  std::string getDeviceId();
  std::string getDeviceName();
  std::string getUserName();

  std::string toJson();
};
}