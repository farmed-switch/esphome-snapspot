#pragma once
#include <string>
#include <vector>
namespace cspot {
struct Context {
  struct ConfigState {
    std::string deviceId;
    std::string deviceName;
    std::string username;
    std::string countryCode;
    std::vector<uint8_t> authData;
    int volume = 0;
  };
  ConfigState config;
};
}
