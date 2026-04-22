#include "AccessKeyFetcher.h"

#include <cstring>
#include <initializer_list>
#include <map>
#include <type_traits>
#include <vector>

#include "BellLogger.h"
#include "CSpotContext.h"
#include "HTTPClient.h"
#include "Logger.h"
#include "MercurySession.h"
#include "NanoPBExtensions.h"
#include "NanoPBHelper.h"
#include "Packet.h"
#include "TimeProvider.h"
#include "Utils.h"

#ifdef BELL_ONLY_CJSON
#include "cJSON.h"
#else
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#endif

#include "protobuf/login5.pb.h"

using namespace cspot;

static std::string CLIENT_ID =
    "65b708073fc0480ea92a077233ca87bd";

AccessKeyFetcher::AccessKeyFetcher(std::shared_ptr<cspot::Context> ctx)
    : ctx(ctx) {}

bool AccessKeyFetcher::isExpired() {
  if (accessKey.empty()) {
    return true;
  }

  if (ctx->timeProvider->getSyncedTimestamp() > expiresAt) {
    return true;
  }

  return false;
}

std::string AccessKeyFetcher::getAccessKey() {
  if (!isExpired()) {
    return accessKey;
  }

  updateAccessKey();

  return accessKey;
}

void AccessKeyFetcher::updateAccessKey() {
  if (keyPending) {

    return;
  }

  keyPending = true;

  LoginRequest loginRequest = LoginRequest_init_zero;
  LoginResponse loginResponse = LoginResponse_init_zero;

  loginRequest.client_info.client_id.funcs.encode = &bell::nanopb::encodeString;
  loginRequest.client_info.client_id.arg = &CLIENT_ID;

  loginRequest.client_info.device_id.funcs.encode = &bell::nanopb::encodeString;
  loginRequest.client_info.device_id.arg = &ctx->config.deviceId;

  loginRequest.login_method.stored_credential.username.funcs.encode =
      &bell::nanopb::encodeString;
  loginRequest.login_method.stored_credential.username.arg =
      &ctx->config.username;

  loginRequest.which_login_method = LoginRequest_stored_credential_tag;
  loginRequest.login_method.stored_credential.data.funcs.encode =
      &bell::nanopb::encodeVector;
  loginRequest.login_method.stored_credential.data.arg = &ctx->config.authData;

  int retryCount = 3;
  bool success = false;

  do {
    auto encodedRequest = pbEncode(LoginRequest_fields, &loginRequest);
    CSPOT_LOG(info, "Access token expired, fetching new one... %d",
              encodedRequest.size());

    auto response = bell::HTTPClient::post(
        "https://login5.spotify.com/v3/login",
        {{"Content-Type", "application/x-protobuf"}}, encodedRequest);

    if (!response) {
      CSPOT_LOG(error, "login5 request failed (null response)");
      retryCount--;
      continue;
    }
    auto responseBytes = response->bytes();
    if (responseBytes.empty()) {
      CSPOT_LOG(error, "login5 returned empty body");
      retryCount--;
      continue;
    }

    pbDecode(loginResponse, LoginResponse_fields, responseBytes);

    if (loginResponse.which_response == LoginResponse_ok_tag) {

      CSPOT_LOG(info, "Access token sucessfully fetched");
      success = true;

      accessKey = std::string(loginResponse.response.ok.access_token);

      int expiresIn = 3600 / 2;

      if (loginResponse.response.ok.has_access_token_expires_in) {
        expiresIn = loginResponse.response.ok.access_token_expires_in / 2;
      }

      CSPOT_LOG(info, "Access token expires in %d s (half-life)", expiresIn);
      this->expiresAt =
          ctx->timeProvider->getSyncedTimestamp() + (expiresIn * 1000);
    } else {
      CSPOT_LOG(error, "Failed to fetch access token");
    }

    pb_release(LoginResponse_fields, &loginResponse);

    retryCount--;
  } while (retryCount >= 0 && !success);

  keyPending = false;
}
