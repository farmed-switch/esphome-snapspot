#pragma once
#include <string>
#include <vector>
namespace cspot {
class LoginBlob {
 public:
  explicit LoginBlob(std::string name) {}
  virtual ~LoginBlob() = default;
  std::vector<uint8_t> authData;
  std::string username;
  int authType = 0;
};
}
