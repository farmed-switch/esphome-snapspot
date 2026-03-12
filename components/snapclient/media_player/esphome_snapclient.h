#pragma once

#include "esphome/core/defines.h"

#ifdef USE_ESP32
#ifndef USE_I2S_LEGACY

#include <atomic>
#include <string>

// Set to true by spotify_connect when it wants the I2S bus.
// Snapclient loop() checks this and yields by pausing its player.
extern std::atomic<bool> g_snapclient_preempted;

#include "esphome/core/component.h"
#include "esphome/components/media_player/media_player.h"
#include "esphome/components/i2s_audio/i2s_audio.h"
#include "esphome/components/priority_lock/priority_lock.h"
#include "esphome/core/gpio.h"
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#include "player.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef USE_AUDIO_DAC
#include "esphome/components/audio_dac/audio_dac.h"
#else
namespace esphome::audio_dac { class AudioDac; }
#endif

namespace esphome::snapclient {

using audioDACdata_t = struct AudioDaCdataS {
  bool playerMute;
  bool stateMute;
  int volume;
};

static const char *const TAG = "snapclient";

class SnapClientComponent : public i2s_audio::I2SAudioOut, public media_player::MediaPlayer, public Component {
 public:
  void set_mute_pin(GPIOPin *mute_pin) { this->mute_pin_ = mute_pin; }
  void setup() override;
  void loop() override;
  void on_shutdown() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }
  void set_dout_pin(InternalGPIOPin *pin) { this->dout_pin_ = pin; }
  void set_snapserver_hostname(const std::string &hostname) { this->snapserver_hostname_ = hostname; }
  void set_snapserver_port(uint16_t port) { this->snapserver_port_ = port; }
  void set_snapserver_use_mdns(bool use_mdns) { this->snapserver_use_mdns_ = use_mdns; }
  void set_snapcast_min_db(float db) { this->snapcast_min_db_ = db; }
  void set_snapcast_max_db(float db) { this->snapcast_max_db_ = db; }
  void set_priority(uint8_t p) { this->priority_ = p; }
  void set_priority_lock(priority_lock::PriorityLockManager *lock) { this->priority_lock_ = lock; }
#ifdef USE_TEXT_SENSOR
  void set_track_name_sensor(text_sensor::TextSensor *s) { this->track_name_sensor_ = s; }
  void set_artist_sensor(text_sensor::TextSensor *s) { this->artist_sensor_ = s; }
  void set_album_sensor(text_sensor::TextSensor *s) { this->album_sensor_ = s; }
  void set_album_art_url_sensor(text_sensor::TextSensor *s) { this->album_art_url_sensor_ = s; }
#endif
#ifdef USE_SENSOR
  void set_duration_sensor(sensor::Sensor *s) { this->duration_sensor_ = s; }
  void set_position_sensor(sensor::Sensor *s) { this->position_sensor_ = s; }
#endif
#ifdef USE_AUDIO_DAC
  void set_audio_dac(audio_dac::AudioDac *audio_dac) { this->audio_dac_ = audio_dac; }
#endif

  media_player::MediaPlayerTraits get_traits() override;
  bool is_muted() const override { return this->mute_state_; }

  void set_mute_from_isr(bool mute, bool set_state);
  void set_volume_from_isr(int volume);
  SemaphoreHandle_t playerStateChangedMutex;
  player_state_e player_state{IDLE};

 protected:
  media_player::MediaPlayerState get_state_from_player_state_(player_state_e state);
  void control(const media_player::MediaPlayerCall &call) override;

  void set_mute_(bool mute);
  // push_server=false during boot/NVS init so we don't overwrite the
  // Snapserver's current volume with the locally-cached NVS value.
  void set_volume_(float volume, bool publish = true, bool push_server = true);

  // Push volume/mute changes from HA to the Snapserver via JSON-RPC.
  void push_volume_to_server_(float volume);
  void push_mute_to_server_(bool mute);
  static void http_rpc_task_(void *pvParam);

  void dac_control_();
  bool has_lock_{false};
  // UPPGIFT 7F: handle tracked so unlock_() can call i2s_del_channel() if needed.
  // player_task already calls i2s_del_channel() before firing the state callback,
  // so this remains nullptr by the time unlock_() runs — the guard is a safety net.
  i2s_chan_handle_t i2s_channel_handle_{nullptr};
  bool lock_() {
    if (this->has_lock_) return true;
    // Respect active preemption: Spotify has claimed (or is about to claim)
    // the I2S bus. This prevents the 23ms re-acquisition window that occurs
    // when a state transition fires lock_() as a side-effect right after
    // unlock_() but before Spotify has grabbed parent_->try_lock().
    if (g_snapclient_preempted.load(std::memory_order_relaxed)) return false;
    // Go through priority_lock when available so the lock manager
    // tracks ownership — regardless of caller (setup, control, player_task).
    if (this->priority_lock_) {
      if (!this->priority_lock_->try_acquire("snapclient"))
        return false;
    }
    if (!this->parent_->try_lock()) {
      if (this->priority_lock_) this->priority_lock_->release("snapclient");
      return false;
    }
    this->has_lock_ = true;
    return true;
  }
  void unlock_();
  float volume_{0.0f};  // init 0 (silent), loaded from NVS in setup()
  bool mute_state_{true};
  GPIOPin *mute_pin_{nullptr};
  InternalGPIOPin *dout_pin_{nullptr};
  std::string snapserver_hostname_{};
  uint16_t snapserver_port_{1704};
  bool snapserver_use_mdns_{true};
  bool network_initialized_{false};
  float snapcast_min_db_{-45.0f};
  float snapcast_max_db_{15.0f};
  uint8_t priority_{50};
  priority_lock::PriorityLockManager *priority_lock_{nullptr};
  void on_preempt_();
  void on_resume_();
  audioDACdata_t dac_data_;
  audioDACdata_t dac_data_external_;
  SemaphoreHandle_t audio_dac_semaphore_;
  QueueHandle_t audio_q_hdl_;

  // Volume sync: debounce timestamps (millis())
  uint32_t last_ha_volume_ms_{0};      // when HA last pushed to Snapserver
  uint32_t last_server_volume_ms_{0};  // when Snapserver last pushed to us

  // HTTP RPC worker task for HA→Snapserver volume push.
  struct rpc_request_t {
    int  percent;  // 0-100
    bool muted;
  };
  QueueHandle_t http_rpc_queue_{nullptr};
  TaskHandle_t  http_rpc_task_handle_{nullptr};
#ifdef USE_TEXT_SENSOR
  friend void on_snapcast_meta(const char *, const char *, const char *, void *);
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
  audio_dac::AudioDac *audio_dac_{nullptr};
};

}  // namespace esphome::snapclient

#endif
#endif
