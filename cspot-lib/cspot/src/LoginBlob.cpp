#include "LoginBlob.h"

#include <stdio.h>
#include <initializer_list>

#include "BellLogger.h"
#include "ConstantParameters.h"
#include "Logger.h"
#include "protobuf/authentication.pb.h"
#ifdef BELL_ONLY_CJSON
#include "cJSON.h"
#else
#include "nlohmann/detail/json_pointer.hpp"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#endif

using namespace cspot;

LoginBlob::LoginBlob(std::string name) {
  char hash[32];
  sprintf(hash, "%016zu", std::hash<std::string>{}(name));

  this->deviceId = std::string("142137fd329622137a149016") + std::string(hash);
  this->crypto = std::make_unique<Crypto>();
  this->name = name;

  this->crypto->dhInit();
}

std::vector<uint8_t> LoginBlob::decodeBlob(
    const std::vector<uint8_t>& blob, const std::vector<uint8_t>& sharedKey) {

  if (blob.size() < 37) {
    CSPOT_LOG(error, "Blob too short!");
    return std::vector<uint8_t>();
  }

  if (sharedKey.empty()) {
    CSPOT_LOG(error, "SharedKey is empty!");
    return std::vector<uint8_t>();
  }

  auto iv = std::vector<uint8_t>(blob.begin(), blob.begin() + 16);
  auto encrypted = std::vector<uint8_t>(blob.begin() + 16, blob.end() - 20);
  auto checksum = std::vector<uint8_t>(blob.end() - 20, blob.end());

  crypto->sha1Init();

  crypto->sha1Update(sharedKey);
  auto baseKey = crypto->sha1FinalBytes();
  baseKey = std::vector<uint8_t>(baseKey.begin(), baseKey.begin() + 16);

  auto checksumMessage = std::string("checksum");
  auto checksumKey = crypto->sha1HMAC(
      baseKey,
      std::vector<uint8_t>(checksumMessage.begin(), checksumMessage.end()));

  auto encryptionMessage = std::string("encryption");
  auto encryptionKey = crypto->sha1HMAC(
      baseKey,
      std::vector<uint8_t>(encryptionMessage.begin(), encryptionMessage.end()));

  auto mac = crypto->sha1HMAC(checksumKey, encrypted);

  if (mac != checksum) {
    CSPOT_LOG(error, "Mac doesn't match — blob corrupt, aborting");
    return std::vector<uint8_t>();
  }

  encryptionKey =
      std::vector<uint8_t>(encryptionKey.begin(), encryptionKey.begin() + 16);
  crypto->aesCTRXcrypt(encryptionKey, iv, encrypted.data(), encrypted.size());

  return encrypted;
}

uint32_t LoginBlob::readBlobInt(const std::vector<uint8_t>& data) {
  auto lo = data[blobSkipPosition];
  if ((int)(lo & 0x80) == 0) {
    this->blobSkipPosition += 1;
    return lo;
  }

  auto hi = data[blobSkipPosition + 1];
  this->blobSkipPosition += 2;

  return (uint32_t)((lo & 0x7f) | (hi << 7));
}

std::vector<uint8_t> LoginBlob::decodeBlobSecondary(
    const std::vector<uint8_t>& blob, const std::string& username,
    const std::string& deviceId) {

  if (blob.empty()) {
    CSPOT_LOG(error, "decodeBlobSecondary: blob is empty!");
    return std::vector<uint8_t>();
  }

  if (username.empty() || deviceId.empty()) {
    CSPOT_LOG(error, "decodeBlobSecondary: username or deviceId is empty!");
    return std::vector<uint8_t>();
  }

  auto encryptedString = std::string(blob.begin(), blob.end());
  auto blobData = crypto->base64Decode(encryptedString);

  if (blobData.empty()) {
    CSPOT_LOG(error, "decodeBlobSecondary: base64 decode failed!");
    return std::vector<uint8_t>();
  }

  crypto->sha1Init();
  crypto->sha1Update(std::vector<uint8_t>(deviceId.begin(), deviceId.end()));
  auto secret = crypto->sha1FinalBytes();
  auto pkBaseKey = crypto->pbkdf2HmacSha1(
      secret, std::vector<uint8_t>(username.begin(), username.end()), 256, 20);

  crypto->sha1Init();
  crypto->sha1Update(pkBaseKey);
  auto key = std::vector<uint8_t>({0x00, 0x00, 0x00, 0x14});
  auto baseKeyHashed = crypto->sha1FinalBytes();
  key.insert(key.begin(), baseKeyHashed.begin(), baseKeyHashed.end());

  crypto->aesECBdecrypt(key, blobData);

  if (blobData.size() < 17) {
    CSPOT_LOG(error, "decodeBlobSecondary: blob too short after decrypt (%zu bytes)", blobData.size());
    return std::vector<uint8_t>();
  }

  auto l = blobData.size();

  for (int i = 0; i < l - 16; i++) {
    blobData[l - i - 1] ^= blobData[l - i - 17];
  }

  return blobData;
}

void LoginBlob::loadZeroconf(const std::vector<uint8_t>& blob,
                             const std::vector<uint8_t>& sharedKey,
                             const std::string& deviceId,
                             const std::string& username) {

  auto partDecoded = this->decodeBlob(blob, sharedKey);
  if (partDecoded.empty()) {
    CSPOT_LOG(error, "loadZeroconf: decodeBlob failed");
    return;
  }
  auto loginData = this->decodeBlobSecondary(partDecoded, username, deviceId);
  if (loginData.empty()) {
    CSPOT_LOG(error, "loadZeroconf: decodeBlobSecondary failed");
    return;
  }

  blobSkipPosition = 1;
  blobSkipPosition += readBlobInt(loginData);
  blobSkipPosition += 1;
  this->authType = readBlobInt(loginData);
  blobSkipPosition += 1;
  auto authSize = readBlobInt(loginData);
  this->username = username;
  this->authData =
      std::vector<uint8_t>(loginData.begin() + blobSkipPosition,
                           loginData.begin() + blobSkipPosition + authSize);
}

void LoginBlob::loadUserPass(const std::string& username,
                             const std::string& password) {
  this->username = username;
  this->authData = std::vector<uint8_t>(password.begin(), password.end());
  this->authType =
      static_cast<uint32_t>(AuthenticationType_AUTHENTICATION_USER_PASS);
}

void LoginBlob::loadJson(const std::string& json) {
#ifdef BELL_ONLY_CJSON
  cJSON* root = cJSON_Parse(json.c_str());
  this->authType = cJSON_GetObjectItem(root, "authType")->valueint;
  this->username = cJSON_GetObjectItem(root, "username")->valuestring;
  std::string authDataObject =
      cJSON_GetObjectItem(root, "authData")->valuestring;
  this->authData = crypto->base64Decode(authDataObject);
  cJSON_Delete(root);
#else
  auto root = nlohmann::json::parse(json);
  this->authType = root["authType"];
  this->username = root["username"];
  std::string authDataObject = root["authData"];

  this->authData = crypto->base64Decode(authDataObject);
#endif
}

std::string LoginBlob::toJson() {
#ifdef BELL_ONLY_CJSON
  cJSON* json_obj = cJSON_CreateObject();
  cJSON_AddStringToObject(json_obj, "authData",
                          crypto->base64Encode(authData).c_str());
  cJSON_AddNumberToObject(json_obj, "authType", this->authType);
  cJSON_AddStringToObject(json_obj, "username", this->username.c_str());

  char* str = cJSON_PrintUnformatted(json_obj);
  cJSON_Delete(json_obj);
  std::string json_objStr(str);
  free(str);

  return json_objStr;
#else
  nlohmann::json obj;
  obj["authData"] = crypto->base64Encode(authData);
  obj["authType"] = this->authType;
  obj["username"] = this->username;

  return obj.dump();
#endif
}

void LoginBlob::loadZeroconfQuery(
    std::map<std::string, std::string>& queryParams) {

  auto username = queryParams["userName"];
  auto blobString = queryParams["blob"];
  auto clientKeyString = queryParams["clientKey"];
  auto deviceName = queryParams["deviceName"];

  if (username.empty() || blobString.empty() || clientKeyString.empty()) {
    CSPOT_LOG(error, "loadZeroconfQuery: Missing required parameters!");
    CSPOT_LOG(error, "  username: %s", username.empty() ? "EMPTY" : "OK");
    CSPOT_LOG(error, "  blob: %s", blobString.empty() ? "EMPTY" : "OK");
    CSPOT_LOG(error, "  clientKey: %s", clientKeyString.empty() ? "EMPTY" : "OK");
    return;
  }

  auto clientKeyBytes = crypto->base64Decode(clientKeyString);
  auto blobBytes = crypto->base64Decode(blobString);

  if (clientKeyBytes.empty() || blobBytes.empty()) {
    CSPOT_LOG(error, "loadZeroconfQuery: base64 decode failed!");
    return;
  }

  if (!crypto) {
    CSPOT_LOG(error, "loadZeroconfQuery: crypto object is NULL!");
    return;
  }

  if (crypto->publicKey.empty()) {
    CSPOT_LOG(error, "loadZeroconfQuery: DH publicKey is empty!");
    return;
  }

  auto secretKey = crypto->dhCalculateShared(clientKeyBytes);

  if (secretKey.empty()) {
    CSPOT_LOG(error, "loadZeroconfQuery: DH shared secret calculation failed!");
    return;
  }

  this->loadZeroconf(blobBytes, secretKey, deviceId, username);
}

std::string LoginBlob::buildZeroconfInfo() {

  auto encodedKey = crypto->base64Encode(crypto->publicKey);
#ifdef BELL_ONLY_CJSON
  cJSON* json_obj = cJSON_CreateObject();
  cJSON_AddNumberToObject(json_obj, "status", 101);
  cJSON_AddStringToObject(json_obj, "statusString", "OK");
  cJSON_AddStringToObject(json_obj, "version", cspot::protocolVersion);
  cJSON_AddStringToObject(json_obj, "libraryVersion", cspot::swVersion);
  cJSON_AddStringToObject(json_obj, "accountReq", "PREMIUM");
  cJSON_AddStringToObject(json_obj, "brandDisplayName", cspot::brandName);
  cJSON_AddStringToObject(json_obj, "modelDisplayName", name.c_str());
  cJSON_AddStringToObject(json_obj, "voiceSupport", "NO");
  cJSON_AddStringToObject(json_obj, "availability", this->username.c_str());
  cJSON_AddNumberToObject(json_obj, "productID", 0);
  cJSON_AddStringToObject(json_obj, "tokenType", "default");
  cJSON_AddStringToObject(json_obj, "groupStatus", "NONE");
  cJSON_AddStringToObject(json_obj, "resolverVersion", "0");
  cJSON_AddStringToObject(json_obj, "scope",
                          "streaming,client-authorization-universal");

  cJSON_AddStringToObject(json_obj, "activeUser", this->username.c_str());
  cJSON_AddStringToObject(json_obj, "deviceID", deviceId.c_str());
  cJSON_AddStringToObject(json_obj, "remoteName", name.c_str());
  cJSON_AddStringToObject(json_obj, "publicKey", encodedKey.c_str());
  cJSON_AddStringToObject(json_obj, "deviceType", "SPEAKER");

  char* str = cJSON_PrintUnformatted(json_obj);
  cJSON_Delete(json_obj);
  std::string json_objStr(str);
  free(str);

  return json_objStr;
#else
  nlohmann::json obj;
  obj["status"] = 101;
  obj["statusString"] = "OK";
  obj["version"] = cspot::protocolVersion;
  obj["spotifyError"] = 0;
  obj["libraryVersion"] = cspot::swVersion;
  obj["accountReq"] = "PREMIUM";
  obj["brandDisplayName"] = cspot::brandName;
  obj["modelDisplayName"] = name;
  obj["voiceSupport"] = "NO";
  obj["availability"] = this->username;
  obj["productID"] = 0;
  obj["tokenType"] = "default";
  obj["groupStatus"] = "NONE";
  obj["resolverVersion"] = "0";
  obj["scope"] = "streaming,client-authorization-universal";

  obj["activeUser"] = this->username;
  obj["deviceID"] = deviceId;
  obj["remoteName"] = name;
  obj["publicKey"] = encodedKey;
  obj["deviceType"] = "SPEAKER";

  return obj.dump();
#endif
}

std::string LoginBlob::getDeviceId() {
  return this->deviceId;
}
std::string LoginBlob::getDeviceName() {
  return this->name;
}
std::string LoginBlob::getUserName() {
  return this->username;
}
