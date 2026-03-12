#pragma once

#include "esphome/core/defines.h"

#ifdef USE_ESP32
#ifndef USE_I2S_LEGACY

#include <string>
#include <map>
#include <vector>

#include "esphome/core/component.h"
#include "esphome/components/media_player/media_player.h"
#include "esphome/components/i2s_audio/i2s_audio.h"
#include "esphome/components/priority_lock/priority_lock.h"
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#include "esphome/core/gpio.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/i2s_std.h"
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

/**
 * SpotifyConnectComponent
 *
 * ESPHome media_player component that wraps the cspot Spotify Connect library.
 * Follows the exact same ESPHome lifecycle and I2S lock pattern as
 * SnapClientComponent (PR #14389), so both components can share one I2S bus.
 *
 * Lock semantics (identical to snapclient):
 *   - setup() acquires the I2S lock; if unavailable the component marks itself failed.
 *   - When the user pauses/stops, unlock_() releases the bus so the other
 *     media_player component (snapclient) can acquire it.
 *   - When the user unpauses, lock_() re-acquires before resuming playback.
 *
 * TODO (full integration, TASK-3+):
 *   - Create I2S channel in setup().
 *   - Start cspot LoginBlob / mDNS advertisement.
 *   - Launch playbackTask (Core 1) and authTask.
 *   - Wire ESPHomeAudioSink::feedPCMFrames() → eq_process_stereo_int16() → I2S.
 *   - Implement graceful shutdown (on_shutdown trigger → stop cspot service).
 */
class SpotifyConnectComponent : public i2s_audio::I2SAudioOut,
                                 public media_player::MediaPlayer,
                                 public Component {
 public:
  /* ---- Config setters (called from Python to_code) ---- */
  void set_dout_pin(InternalGPIOPin *pin) { dout_pin_ = pin; }
  void set_device_name(const std::string &name) { device_name_ = name; }
  void set_priority(uint8_t priority) { priority_ = priority; }
  void set_priority_lock(priority_lock::PriorityLockManager *lock) { priority_lock_ = lock; }
  void set_spotify_min_db(float db) { spotify_min_db_ = db; }
  void set_spotify_max_db(float db) { spotify_max_db_ = db; }
  // TODO (FIX 5 full): void set_priority_lock(priority_lock::PriorityLockManager *lock) { priority_lock_ = lock; }

  /* ---- Metadata sensor setters (wired from Python to_code, all optional) ---- */
#ifdef USE_TEXT_SENSOR
  void set_track_name_sensor(text_sensor::TextSensor *s) { track_name_sensor_ = s; }
  void set_artist_sensor(text_sensor::TextSensor *s) { artist_sensor_ = s; }
  void set_album_sensor(text_sensor::TextSensor *s) { album_sensor_ = s; }
  void set_album_art_url_sensor(text_sensor::TextSensor *s) { album_art_url_sensor_ = s; }
#endif
#ifdef USE_SENSOR
  void set_duration_sensor(sensor::Sensor *s) { duration_sensor_ = s; }
  void set_position_sensor(sensor::Sensor *s) { position_sensor_ = s; }
#endif
#ifdef USE_AUDIO_DAC
  void set_audio_dac(audio_dac::AudioDac *dac) { audio_dac_ = dac; }
#endif

  /* ---- ESPHome Component interface ---- */
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

  /* ---- ESPHome MediaPlayer interface ---- */
  media_player::MediaPlayerTraits get_traits() override;
  bool is_muted() const override { return mute_state_; }

 protected:
  void control(const media_player::MediaPlayerCall &call) override;

  void set_mute_(bool mute);
  void set_volume_(float volume, bool publish = true);
  void on_preempt_();
  void on_resume_();
  void handle_zeroconf_post_(const char *body, int len);
  static void auth_task_wrapper_(void *param);
  void run_auth_task_();
  static void packet_task_wrapper_(void *param);
  static void playback_task_wrapper_(void *param);
  void handle_spirc_event_(std::unique_ptr<cspot::SpircHandler::Event> event);

  /**
   * Acquire the shared I2S bus and create an I2S TX channel for Spotify (44100 Hz S16 stereo).
   * Returns true if the lock was obtained (or was already held).
   */
  bool lock_();

  /**
   * Disable and delete the I2S TX channel, then release the shared I2S bus.
   */
  void unlock_();

  /* ---- State ---- */
  bool has_lock_{false};
  bool network_initialized_{false};
  bool is_playing_{false};
  bool mute_state_{true};
  uint32_t disc_hold_until_ms_{0};   // Block immediate DISC for 5s after PLAYBACK_START
  bool     pending_disc_{false};     // Deferred DISC: set when DISC fires during hold/play
  uint32_t pending_disc_at_ms_{0};   // Timestamp of first pending DISC

  /* ---- Configuration ---- */
  InternalGPIOPin *dout_pin_{nullptr};
  uint8_t priority_{50};
  std::string device_name_{"SnapSpot"};
  float volume_{0.0f};         // init 0 (silent), loaded from NVS in setup()
  float soft_vol_gain_{1.0f};  // PCM multiplier for soft volume (when audio_dac_ is null)
  uint32_t last_ha_volume_ms_{0};     // timestamp of last HA->Spotify push
  uint32_t last_server_volume_ms_{0}; // timestamp of last Spotify->HA update
  float spotify_min_db_{-60.0f};
  float spotify_max_db_{0.0f};
  priority_lock::PriorityLockManager *priority_lock_{nullptr};

  /* ---- cspot integration ---- */
  std::shared_ptr<cspot::LoginBlob> login_blob_;
  std::shared_ptr<cspot::SpircHandler> spirc_handler_;
  std::shared_ptr<cspot::Context> cspot_context_;
  std::unique_ptr<bell::CircularBuffer> audio_buffer_;
  TaskHandle_t playback_task_handle_{nullptr};
  TaskHandle_t packet_task_handle_{nullptr};
  // Auth task static memory -- stored so we can free before next reconnect.
  // Static FreeRTOS tasks do NOT free stack/TCB on vTaskDelete; must be done
  // by the application. Freed at start of handle_zeroconf_post_() before each
  // new auth session to prevent 16 KB internal-RAM leak per reconnect.
  StackType_t  *auth_task_stack_{nullptr};
  StaticTask_t *auth_task_buf_{nullptr};
  httpd_handle_t http_server_{nullptr};
  bool mdns_registered_{false};
  bool auth_in_progress_{false};
  std::string last_auth_username_;  // Prevent duplicate auth with same credentials
  std::map<std::string, std::string> pending_auth_params_;
  i2s_chan_handle_t i2s_tx_handle_{nullptr};

  audio_dac::AudioDac *audio_dac_{nullptr};

  /* ---- Metadata sensors (optional, set from Python to_code) ---- */
#ifdef USE_TEXT_SENSOR
  text_sensor::TextSensor *track_name_sensor_{nullptr};
  text_sensor::TextSensor *artist_sensor_{nullptr};
  text_sensor::TextSensor *album_sensor_{nullptr};
  text_sensor::TextSensor *album_art_url_sensor_{nullptr};
  QueueHandle_t meta_q_hdl_{nullptr};
#endif
#ifdef USE_SENSOR
  sensor::Sensor *duration_sensor_{nullptr};
  sensor::Sensor *position_sensor_{nullptr};
#endif

  /* ---- Metadata state ---- */
  std::string current_track_;
  std::string current_artist_;
  std::string current_album_;
  std::string current_image_url_;
  uint32_t current_duration_ms_{0};
  uint32_t media_position_ms_{0};
  uint32_t media_position_updated_at_{0};
  uint32_t last_position_publish_ms_{0};
};

}  // namespace esphome::spotify_connect

#endif  // USE_I2S_LEGACY
#endif  // USE_ESP32
