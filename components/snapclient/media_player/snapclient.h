#pragma once

#ifdef USE_ESP32
#ifndef USE_I2S_LEGACY

#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/components/media_player/media_player.h"
#include "esphome/components/speaker/speaker.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/gpio.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <atomic>

#ifdef USE_AUDIO_DAC
#include "esphome/components/audio_dac/audio_dac.h"
#else
namespace esphome::audio_dac { class AudioDac; }
#endif

namespace esphome {
namespace snapclient {

static const char *const TAG = "snapclient";

class SnapClientComponent : public media_player::MediaPlayer, public Component {
 public:
  SnapClientComponent() { this->state = media_player::MEDIA_PLAYER_STATE_IDLE; }
  void set_source_speaker(esphome::speaker::Speaker *src) { this->source_ = src; }
  void set_mute_pin(GPIOPin *mute_pin) { this->mute_pin_ = mute_pin; }
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }
#ifdef USE_AUDIO_DAC
  void set_audio_dac(audio_dac::AudioDac *dac) { this->audio_dac_ = dac; }
#endif
  void set_config(std::string name, std::string host, int port) {
    this->name_ = std::move(name);
    this->host_ = std::move(host);
    this->port_ = port;
  }

  void set_snapcast_min_db(float db) { this->snapcast_min_db_ = db; }
  void set_snapcast_max_db(float db) { this->snapcast_max_db_ = db; }
  void set_min_volume(float v) { this->min_volume_pct_ = v / 100.0f; }
  void set_max_volume(float v) { this->max_volume_pct_ = v / 100.0f; }

  void set_track_name_sensor(text_sensor::TextSensor *s) { this->track_name_sensor_ = s; }
  void set_artist_sensor(text_sensor::TextSensor *s) { this->artist_sensor_ = s; }
  void set_album_sensor(text_sensor::TextSensor *s) { this->album_sensor_ = s; }
  void set_album_art_url_sensor(text_sensor::TextSensor *s) { this->album_art_url_sensor_ = s; }
  void set_duration_sensor(sensor::Sensor *s) { this->duration_sensor_ = s; }
  void set_position_sensor(sensor::Sensor *s) { this->position_sensor_ = s; }

  void set_sync_age_sensor(sensor::Sensor *s) { sync_age_sensor_ = s; has_any_tele_ = true; }
  void set_short_median_sensor(sensor::Sensor *s) { short_median_sensor_ = s; has_any_tele_ = true; }
  void set_mini_median_sensor(sensor::Sensor *s) { mini_median_sensor_ = s; has_any_tele_ = true; }
  void set_queue_depth_sensor(sensor::Sensor *s) { queue_depth_sensor_ = s; has_any_tele_ = true; }
  void set_heap_free_sensor(sensor::Sensor *s) { heap_free_sensor_ = s; has_any_tele_ = true; }
  void set_psram_free_sensor(sensor::Sensor *s) { psram_free_sensor_ = s; has_any_tele_ = true; }
  void set_decoder_recv_sensor(sensor::Sensor *s) { decoder_recv_sensor_ = s; has_any_tele_ = true; }
  void set_decoder_decoded_sensor(sensor::Sensor *s) { decoder_decoded_sensor_ = s; has_any_tele_ = true; }
  void set_decoder_drop_sensor(sensor::Sensor *s) { decoder_drop_sensor_ = s; has_any_tele_ = true; }
  void set_drift_sensor(sensor::Sensor *s) { drift_sensor_ = s; has_any_tele_ = true; }

  media_player::MediaPlayerTraits get_traits() override;
  bool is_muted() const override { return this->mute_state_; }

  esphome::speaker::Speaker *source_{nullptr};

  void snapclient_stop_begin();
  bool snapclient_tasks_stopped();
  void snapclient_stop_finalize();
  void snapclient_start();
  TaskHandle_t http_get_task_handle_{nullptr};

  bool has_any_tele_{false};
  volatile int64_t  tele_age_us_{0};
  volatile int64_t  tele_short_median_us_{0};
  volatile int64_t  tele_mini_median_us_{0};
  volatile int      tele_queue_depth_{0};
  volatile uint32_t tele_heap_{0};
  volatile uint32_t tele_psram_{0};
  volatile uint32_t tele_recv_{0};
  volatile uint32_t tele_decoded_{0};
  volatile uint32_t tele_drop_{0};
  volatile bool     tele_fresh_{false};

 protected:
  void control(const media_player::MediaPlayerCall &call) override;

  void set_mute_(bool mute);
  void set_volume_(float volume, bool publish = true, bool push_server = true);

  void push_volume_to_server_(float volume);
  void push_mute_to_server_(bool mute);
  static void http_rpc_task_(void *pvParam);

  static void tcp_notify_task_(void *pvParam);

  static void net_monitor_task_(void *pvParam);

  float map_volume_to_eq_gain_(float vol);

  std::string name_;
  std::string host_;
  int port_{1704};
  float snapcast_min_db_{-45.0f};
  float snapcast_max_db_{0.0f};
  float min_volume_pct_{0.0f};
  float max_volume_pct_{1.0f};

  std::atomic<bool> stop_requested_{false};
  bool snapclient_running_{false};

  enum class GateState : uint8_t {
    IDLE,
    STOPPING,
    CLIENT_ACTIVE,
    DISCONNECTING,
    FINALIZING,
  };
  GateState gate_state_{GateState::IDLE};
  uint32_t gate_ts_{0};
  float volume_{0.0f};
  bool mute_state_{true};
  bool network_initialized_{false};
  media_player::MediaPlayerState last_state_{media_player::MEDIA_PLAYER_STATE_IDLE};
  QueueHandle_t audio_q_hdl_{nullptr};
  GPIOPin *mute_pin_{nullptr};
  audio_dac::AudioDac *audio_dac_{nullptr};

  uint32_t last_ha_volume_ms_{0};
  uint32_t last_server_volume_ms_{0};

  uint32_t boot_push_at_ms_{0};
  float boot_volume_{0.5f};

  struct rpc_request_t {
    int  percent;
    bool muted;
  };
  QueueHandle_t http_rpc_queue_{nullptr};
  TaskHandle_t  http_rpc_task_handle_{nullptr};

  TaskHandle_t  tcp_notify_task_handle_{nullptr};
  int           tcp_notify_sock_{-1};
  uint32_t      tcp_last_lock_ms_{0};

  TaskHandle_t  net_monitor_task_handle_{nullptr};

  text_sensor::TextSensor *track_name_sensor_{nullptr};
  text_sensor::TextSensor *artist_sensor_{nullptr};
  text_sensor::TextSensor *album_sensor_{nullptr};
  text_sensor::TextSensor *album_art_url_sensor_{nullptr};
  QueueHandle_t meta_q_hdl_{nullptr};
  sensor::Sensor *duration_sensor_{nullptr};
  sensor::Sensor *position_sensor_{nullptr};

  float    pos_current_s_{0.0f};
  uint32_t pos_last_tick_ms_{0};

  char dedup_title_[96]{};
  char dedup_artist_[64]{};

  sensor::Sensor *sync_age_sensor_{nullptr};
  sensor::Sensor *short_median_sensor_{nullptr};
  sensor::Sensor *mini_median_sensor_{nullptr};
  sensor::Sensor *queue_depth_sensor_{nullptr};
  sensor::Sensor *heap_free_sensor_{nullptr};
  sensor::Sensor *psram_free_sensor_{nullptr};
  sensor::Sensor *decoder_recv_sensor_{nullptr};
  sensor::Sensor *decoder_decoded_sensor_{nullptr};
  sensor::Sensor *decoder_drop_sensor_{nullptr};
  sensor::Sensor *drift_sensor_{nullptr};
};

}
}

#endif
#endif
