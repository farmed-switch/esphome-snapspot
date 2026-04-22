#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>

namespace cspot {
struct Context;

class AccessKeyFetcher {
 public:
  AccessKeyFetcher(std::shared_ptr<cspot::Context> ctx);

  bool isExpired();

  std::string getAccessKey();

  void updateAccessKey();

 private:
  std::shared_ptr<cspot::Context> ctx;

  std::atomic<bool> keyPending = false;
  std::string accessKey;
  long long int expiresAt;
};
}
