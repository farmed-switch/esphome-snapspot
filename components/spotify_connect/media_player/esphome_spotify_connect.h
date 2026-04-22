#pragma once

#include "esphome/core/defines.h"

#ifdef USE_ESP32
#ifndef USE_I2S_LEGACY

#include <string>
#include <map>
#include <vector>
#include <atomic>

#include "esphome/core/component.h"
#include "esphome/components/media_player/media_player.h"
#include "esphome/components/speaker/speaker.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/gpio.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "CSpotContext.h"
#include "SpircHandler.h"
#include "TrackPlayer.h"
#include "LoginBlob.h"
#include "BellLogger.h"
#include "CircularBuffer.h"

#ifdef USE_AUDIO_DAC
#include "esphome/components/audio_dac/audio_dac.h"
#else
namespace esphome::audio_dac { class AudioDac; }
#endif

namespace esphome::spotify_connect {

static const char *const TAG = "spotify_connect";

class SpotifyConnectComponent : public media_player::MediaPlayer,
                                public Component {
 public:

  void set_source_speaker(esphome::speaker::Speaker *src) { source_ = src; }
  void set_device_name(const std::string &name) { device_name_ = name; }
  void set_spotify_min_db(float db) { spotify_min_db_ = db; }
  void set_spotify_max_db(float db) { spotify_max_db_ = db; }
  void set_min_volume(float v) { min_volume_pct_ = v / 100.0f; }
  void set_max_volume(float v) { max_volume_pct_ = v / 100.0f; }

  void set_track_name_sensor(text_sensor::TextSensor *s) { track_name_sensor_ = s; }
  void set_artist_sensor(text_sensor::TextSensor *s) { artist_sensor_ = s; }
  void set_album_sensor(text_sensor::TextSensor *s) { album_sensor_ = s; }
  void set_album_art_url_sensor(text_sensor::TextSensor *s) { album_art_url_sensor_ = s; }
  void set_duration_sensor(sensor::Sensor *s) { duration_sensor_ = s; }
  void set_position_sensor(sensor::Sensor *s) { position_sensor_ = s; }
#ifdef USE_AUDIO_DAC
  void set_audio_dac(audio_dac::AudioDac *dac) { audio_dac_ = dac; }
#endif

  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

  media_player::MediaPlayerTraits get_traits() override;
  bool is_muted() const override { return mute_state_; }

  void close_connection_impl();
  bool are_tasks_exited_impl();
  void finalize_cleanup_impl();
  void force_cleanup_impl();

 protected:
  void control(const media_player::MediaPlayerCall &call) override;

  void set_mute_(bool mute);
  void set_volume_(float volume, bool publish = true);
  void handle_zeroconf_post_(const char *body, int len);
  static void auth_task_wrapper_(void *param);
  void run_auth_task_();
  static void packet_task_wrapper_(void *param);
  static void playback_task_wrapper_(void *param);
  void handle_spirc_event_(std::unique_ptr<cspot::SpircHandler::Event> event);

  bool network_initialized_{false};
  bool is_playing_{false};
  bool draining_{false};
  bool mute_state_{true};
  bool cspot_format_set_{false};
  uint32_t disc_hold_until_ms_{0};
  bool     pending_disc_{false};
  uint32_t pending_disc_at_ms_{0};

  std::atomic<bool> pending_shutdown_{false};
  bool disconnect_called_{false};
  std::atomic<bool> pkt_task_exited_{true};
  std::atomic<bool> play_task_exited_{true};
  uint32_t idle_since_ms_{0};

  esphome::speaker::Speaker *source_{nullptr};
  std::string device_name_{"SnapSpot"};
  float volume_{0.0f};
  uint32_t last_ha_volume_ms_{0};
  uint32_t last_server_volume_ms_{0};
  float spotify_min_db_{-60.0f};
  float spotify_max_db_{0.0f};
  float min_volume_pct_{0.0f};
  float max_volume_pct_{1.0f};

  std::shared_ptr<cspot::LoginBlob> login_blob_;
  std::shared_ptr<cspot::SpircHandler> spirc_handler_;
  std::shared_ptr<cspot::Context> cspot_context_;
  std::unique_ptr<bell::CircularBuffer> audio_buffer_;
  TaskHandle_t playback_task_handle_{nullptr};
  TaskHandle_t packet_task_handle_{nullptr};
  StackType_t  *pkt_task_stack_{nullptr};
  StaticTask_t *pkt_task_buf_{nullptr};
  StackType_t  *play_task_stack_{nullptr};
  StaticTask_t *play_task_buf_{nullptr};
  StackType_t  *auth_task_stack_{nullptr};
  StaticTask_t *auth_task_buf_{nullptr};
  httpd_handle_t http_server_{nullptr};
  bool mdns_registered_{false};
  bool auth_in_progress_{false};
  std::string last_auth_username_;
  std::map<std::string, std::string> pending_auth_params_;

  audio_dac::AudioDac *audio_dac_{nullptr};

  std::atomic<bool> pending_nvs_volume_{false};
  std::atomic<float> pending_nvs_volume_val_{0.0f};

  text_sensor::TextSensor *track_name_sensor_{nullptr};
  text_sensor::TextSensor *artist_sensor_{nullptr};
  text_sensor::TextSensor *album_sensor_{nullptr};
  text_sensor::TextSensor *album_art_url_sensor_{nullptr};
  QueueHandle_t meta_q_hdl_{nullptr};
  sensor::Sensor *duration_sensor_{nullptr};
  sensor::Sensor *position_sensor_{nullptr};

  std::string current_track_;
  std::string current_artist_;
  std::string current_album_;
  std::string current_image_url_;
  uint32_t current_duration_ms_{0};
  uint32_t media_position_ms_{0};
  uint32_t media_position_updated_at_{0};
  uint32_t last_position_publish_ms_{0};
};

}

#endif
#endif
