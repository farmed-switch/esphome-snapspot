#include "ApResolve.h"

#include <initializer_list>
#include <map>
#include <memory>
#include <string_view>
#include <vector>

#include "HTTPClient.h"
#include "Logger.h"
#ifdef BELL_ONLY_CJSON
#include "cJSON.h"
#else
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#endif
#include <esp_random.h>

using namespace cspot;

ApResolve::ApResolve(std::string apOverride) {
  this->apOverride = apOverride;
}

std::string ApResolve::fetchFirstApAddress() {
  if (apOverride != "") {
    return apOverride;
  }

  static const char* FALLBACK_AP = "ap.spotify.com:443";

  auto request = bell::HTTPClient::get("https://apresolve.spotify.com/");
  if (!request) {
    CSPOT_LOG(error, "apresolve: HTTP request failed, using fallback");
    return FALLBACK_AP;
  }

  std::string_view responseStr = request->body();
  if (responseStr.empty()) {
    CSPOT_LOG(error, "apresolve: empty response body, using fallback");
    return FALLBACK_AP;
  }

#ifdef BELL_ONLY_CJSON
  cJSON* json = cJSON_Parse(responseStr.data());
  if (!json) {
    CSPOT_LOG(error, "apresolve: JSON parse failed, using fallback");
    return FALLBACK_AP;
  }
  cJSON* ap_list = cJSON_GetObjectItem(json, "ap_list");
  if (!ap_list || !cJSON_IsArray(ap_list) || cJSON_GetArraySize(ap_list) == 0) {
    CSPOT_LOG(error, "apresolve: ap_list missing or empty, using fallback");
    cJSON_Delete(json);
    return FALLBACK_AP;
  }
  int count = cJSON_GetArraySize(ap_list);
  int index = (count > 1) ? (int)(esp_random() % (unsigned)count) : 0;
  auto ap_string = std::string(cJSON_GetArrayItem(ap_list, index)->valuestring);
  cJSON_Delete(json);
  return ap_string;
#else
  try {
    auto json = nlohmann::json::parse(responseStr);
    auto& ap_list = json["ap_list"];
    int count = (int)ap_list.size();
    if (count == 0) {
      CSPOT_LOG(error, "apresolve: ap_list empty, using fallback");
      return FALLBACK_AP;
    }
    int index = (count > 1) ? (int)(esp_random() % (unsigned)count) : 0;
    return ap_list[index];
  } catch (...) {
    CSPOT_LOG(error, "apresolve: JSON parse exception, using fallback");
    return FALLBACK_AP;
  }
#endif
}
