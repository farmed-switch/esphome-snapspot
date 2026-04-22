#include "Session.h"

#include <limits.h>
#include <cstdint>
#include <functional>
#include <memory>
#include <random>
#include <type_traits>
#include <utility>

#include "ApResolve.h"
#include "AuthChallenges.h"
#include "BellLogger.h"
#include "Logger.h"
#include "LoginBlob.h"
#include "Packet.h"
#include "PlainConnection.h"
#include "ShannonConnection.h"

#include "NanoPBHelper.h"
#include "pb_decode.h"
#include "protobuf/authentication.pb.h"

using random_bytes_engine =
    std::independent_bits_engine<std::default_random_engine, CHAR_BIT, uint8_t>;

using namespace cspot;

Session::Session() {
  this->challenges = std::make_unique<cspot::AuthChallenges>();
}

Session::~Session() {}

void Session::connect(std::unique_ptr<cspot::PlainConnection> connection) {
  this->conn = std::move(connection);
  conn->timeoutHandler = [this]() {
    return this->triggerTimeout();
  };
  auto helloPacket = this->conn->sendPrefixPacket(
      {0x00, 0x04}, this->challenges->prepareClientHello());
  auto apResponse = this->conn->recvPacket();
  CSPOT_LOG(info, "Received APHello response");

  auto solvedHello = this->challenges->solveApHello(helloPacket, apResponse);

  conn->sendPrefixPacket({}, solvedHello);
  CSPOT_LOG(debug, "Received shannon keys");

  this->shanConn = std::make_shared<ShannonConnection>();

  this->shanConn->wrapConnection(this->conn, challenges->shanSendKey,
                                 challenges->shanRecvKey);
}

void Session::connectWithRandomAp() {
  auto apResolver = std::make_unique<ApResolve>("");
  auto conn = std::make_unique<cspot::PlainConnection>();
  conn->timeoutHandler = [this]() {
    return this->triggerTimeout();
  };

  auto apAddr = apResolver->fetchFirstApAddress();

  CSPOT_LOG(debug, "Connecting with AP <%s>", apAddr.c_str());
  conn->connect(apAddr);

  this->connect(std::move(conn));
}

std::vector<uint8_t> Session::authenticate(std::shared_ptr<LoginBlob> blob) {

  authBlob = blob;

  auto data = challenges->prepareAuthPacket(blob->authData, blob->authType,
                                            deviceId, blob->username);

  this->shanConn->sendPacket(LOGIN_REQUEST_COMMAND, data);

  auto packet = this->shanConn->recvPacket();
  switch (packet.command) {
    case AUTH_SUCCESSFUL_COMMAND: {
      APWelcome welcome;
      CSPOT_LOG(debug, "Authorization successful");
      pbDecode(welcome, APWelcome_fields, packet.data);
      return std::vector<uint8_t>(welcome.reusable_auth_credentials.bytes,
                                  welcome.reusable_auth_credentials.bytes +
                                      welcome.reusable_auth_credentials.size);
    }
    case AUTH_DECLINED_COMMAND: {
      CSPOT_LOG(error, "Authorization declined");
      break;
    }
    default:
      CSPOT_LOG(error, "Unknown auth fail code %d", packet.command);
  }

  return std::vector<uint8_t>(0);
}

void Session::close() {
  this->conn->close();
}
