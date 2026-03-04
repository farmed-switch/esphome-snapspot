#pragma once

#ifdef USE_ESP32
#ifndef USE_I2S_LEGACY

#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/components/i2s_audio/i2s_audio.h"
#include "esphome/core/gpio.h"

#ifdef USE_AUDIO_DAC
#include "esphome/components/audio_dac/audio_dac.h"
#endif

#include "audio_source_manager.h"
#include "spotify_client.h"
#include "snapcast_client.h"
#include "esphome/components/wifi/wifi_component.h"
#include "esp_http_server.h"

#define SNAPSPOT_ZEROCONF_PORT 4000

namespace esphome {
namespace snapspot {

static const char *const SNAPSPOT_TAG = "snapspot";

class SnapSpotComponent : public i2s_audio::I2SAudioOut, public Component {
 public:
  void set_mute_pin(GPIOPin *mute_pin) { this->mute_pin_ = mute_pin; }
  void set_dout_pin(uint8_t pin) { this->dout_pin_ = pin; }
  void set_spotify_name(std::string name) { this->spotify_name_ = std::move(name); }
  void set_snapcast_host(std::string host) { this->snapcast_host_ = std::move(host); }
  void set_snapcast_port(int port) { this->snapcast_port_ = port; }
  void set_spotify_volume_max(float v) { this->spotify_volume_max_ = v; }
  void set_snapcast_volume_max(float v) { this->snapcast_volume_max_ = v; }
  void set_dac_db_range(float min_db, float max_db) { dac_min_db_ = min_db; dac_max_db_ = max_db; }
  void set_spotify_db_range(float min_db, float max_db) { spotify_min_db_ = min_db; spotify_max_db_ = max_db; }
  void set_snapcast_db_range(float min_db, float max_db) { snapcast_min_db_ = min_db; snapcast_max_db_ = max_db; }

  // Scale a 0–100% value in a source-specific dB window to 0.0–1.0 for set_volume().
  // Clamps result to [0,1] so TAS5805M never gets out-of-range values.
  static float scaleVolume(float percent, float src_min_db, float src_max_db,
                            float dac_min_db, float dac_max_db) {
    float dac_range = dac_max_db - dac_min_db;
    if (dac_range <= 0.0f) return percent / 100.0f;
    float target_db = src_min_db + (src_max_db - src_min_db) * (percent / 100.0f);
    float v = (target_db - dac_min_db) / dac_range;
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;
    return v;
  }

#ifdef USE_AUDIO_DAC
  void set_audio_dac(audio_dac::AudioDac *audio_dac) { this->audio_dac_ = audio_dac; }
#endif

  void setup() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

  bool get_mute_state() { return this->mute_state_; }

  // Public so static ZeroConf HTTP handlers in .cpp can access it
  spotify::SpotifyClient *spotify_client_{nullptr};

 protected:
  std::string spotify_name_{"SnapSpot"};
  std::string snapcast_host_;
  int snapcast_port_{1704};

  float volume_{1.0f};
  bool mute_state_{true};
  float spotify_volume_max_{1.0f};
  float snapcast_volume_max_{1.0f};
  float dac_min_db_{-25.0f};
  float dac_max_db_{15.0f};
  float spotify_min_db_{-25.0f};
  float spotify_max_db_{15.0f};
  float snapcast_min_db_{-25.0f};
  float snapcast_max_db_{15.0f};
  QueueHandle_t audio_q_hdl_{nullptr};

  uint8_t dout_pin_{0};
  GPIOPin *mute_pin_{nullptr};
  i2s_chan_handle_t tx_chan_{nullptr};  // I2S TX channel — owned by SnapSpotComponent

#ifdef USE_AUDIO_DAC
  audio_dac::AudioDac *audio_dac_{nullptr};
#endif

  sources::AudioSourceManager *source_manager_{nullptr};
  snapcast::SnapcastClient *snapcast_client_{nullptr};
  audio::AudioOutput *snapcast_audio_output_{nullptr};
  bool mdns_registered_{false};        // Registreras i loop() efter WiFi + mDNS är klara
  uint32_t snapcast_reconnect_time_{0}; // Tid för senaste reconnect-försök (ms)
  httpd_handle_t zeroconf_server_{nullptr};  // Dedikerad ZeroConf HTTP-server (port 4000)
};

}  // namespace snapspot
}  // namespace esphome

#endif  // USE_I2S_LEGACY
#endif  // USE_ESP32
