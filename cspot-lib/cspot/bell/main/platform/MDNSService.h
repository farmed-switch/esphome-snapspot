#pragma once

#include <map>
#include <memory>
#include <string>

namespace bell {

class MDNSService {
 public:
  virtual ~MDNSService() {}
  static std::unique_ptr<MDNSService> registerService(
      const std::string& serviceName, const std::string& serviceType,
      const std::string& serviceProto, const std::string& serviceHost,
      int servicePort, const std::map<std::string, std::string> txtData);
  virtual void unregisterService() = 0;
};

}