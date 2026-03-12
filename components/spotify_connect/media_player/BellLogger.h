#pragma once
namespace bell {
class AbstractLogger {
 public:
  virtual ~AbstractLogger() = default;
  bool enableSubmodule = false;
  bool enableTimestamp = false;
  bool shortTime = false;
  bool enableDebugLogs = true;
  bool enableInfoLogs = true;
};
}
