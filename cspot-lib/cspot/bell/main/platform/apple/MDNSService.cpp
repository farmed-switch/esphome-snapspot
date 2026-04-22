#include "MDNSService.h"

#include <stddef.h>
#include <utility>

#include "dns_sd.h"
#include "i386/endian.h"

using namespace bell;

class implMDNSService : public MDNSService {
 private:
  DNSServiceRef* service;

 public:
  implMDNSService(DNSServiceRef* service) : service(service) {}
  void unregisterService() { DNSServiceRefDeallocate(*service); }
};

std::unique_ptr<MDNSService> MDNSService::registerService(
    const std::string& serviceName, const std::string& serviceType,
    const std::string& serviceProto, const std::string& serviceHost,
    int servicePort, const std::map<std::string, std::string> txtData) {
  DNSServiceRef* ref = new DNSServiceRef();
  TXTRecordRef txtRecord;
  TXTRecordCreate(&txtRecord, 0, NULL);
  for (auto& data : txtData) {
    TXTRecordSetValue(&txtRecord, data.first.c_str(), data.second.size(),
                      data.second.c_str());
  }
  DNSServiceRegister(ref,
                     0,
                     0,
                     serviceName.c_str(),
                     (serviceType + "." + serviceProto)
                         .c_str(),
                     NULL,
                     NULL,
                     htons(servicePort),
                     TXTRecordGetLength(&txtRecord),
                     TXTRecordGetBytesPtr(&txtRecord),
                     NULL,
                     NULL
  );
  TXTRecordDeallocate(&txtRecord);
  return std::make_unique<implMDNSService>(ref);
}
