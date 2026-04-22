#pragma once
#include <stdint.h>
#include <functional>
#include <string>

#include "TCPSocket.h"
#include "mqtt.h"

namespace bell {

class MQTTClient {
 public:
  MQTTClient(){};
  ~MQTTClient() = default;

  enum class QOS {
    AT_MOST_ONCE = MQTT_PUBLISH_QOS_0,
    AT_LEAST_ONCE = MQTT_PUBLISH_QOS_1,
    EXACTLY_ONCE = MQTT_PUBLISH_QOS_2
  };

  typedef std::function<void(const std::string& topic,
                             const std::string& message)>
      PublishCallback;

  void setPublishCallback(PublishCallback callback) {
    _publishCallback = callback;
  }

  void connect(const std::string& host, uint16_t port,
               const std::string& username = "",
               const std::string& password = "");

  void disconnect();

  void sync();

  void publish(const std::string& topic, const std::string& message,
               QOS qos = QOS::AT_MOST_ONCE);

  void subscribe(const std::string& topic, QOS qos = QOS::AT_MOST_ONCE);

  void unsubscribe(const std::string& topic);

  bool isConnected();

  void publishCallback(struct mqtt_response_publish* published);

 private:
  bell::TCPSocket socket;
  bool connected = false;
  PublishCallback _publishCallback = nullptr;

  struct mqtt_client client;
  uint8_t sendbuf[2048];
  uint8_t recvbuf[1024];
};
}
