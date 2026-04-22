#ifdef USE_ESP32
#ifndef USE_I2S_LEGACY

#include "snapclient.h"
#include "decoder.h"
#include "esphome/core/log.h"
#include "esphome/components/network/util.h"
#include "esphome/components/media_player/media_player.h"
#include "esphome/components/audio/audio.h"
#include "esphome/components/snapspot/cspot_gate.h"
#include <cmath>
#ifdef USE_WIFI
#include "esphome/components/wifi/wifi_component.h"
#endif
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"

extern "C" void player_request_resync(void);
extern "C" double player_get_latency_drift(void);

#if CONFIG_USE_DSP_PROCESSOR
#include "dsp_processor.h"
#endif

#ifdef USE_SHARED_AUDIO_EQ
#include "esphome/components/snapspot/shared_audio_eq.h"
#endif

#include "player.h"
#include "snapcast.h"
#ifdef CONFIG_WEB_PORT
#include "ui_http_server.h"
#endif

#include "nvs_flash.h"
#include "nvs.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/tcp.h"
#include <cJSON.h>

static volatile int64_t s_wifi_disc_time_us = 0;
static volatile int64_t s_wifi_conn_time_us = 0;
static volatile int64_t s_ip_got_time_us    = 0;
static volatile int64_t s_ip_lost_time_us   = 0;
static volatile uint32_t s_wifi_disc_count  = 0;
static volatile uint32_t s_wifi_conn_count  = 0;

extern "C" void snapclient_fr_wifi(uint16_t evt_id, int connected);

static void net_wifi_event_handler_(void *  , esp_event_base_t base,
                                    int32_t id, void *  ) {
  int64_t now = esp_timer_get_time();
  if (base == WIFI_EVENT) {
    if (id == WIFI_EVENT_STA_DISCONNECTED) {
      s_wifi_disc_time_us = now;
      s_wifi_disc_count++;
      ESP_LOGW("net_mon", "WiFi DISCONNECTED (#%lu)", (unsigned long)s_wifi_disc_count);
      snapclient_fr_wifi((uint16_t)id, 0);
    } else if (id == WIFI_EVENT_STA_CONNECTED) {
      s_wifi_conn_time_us = now;
      s_wifi_conn_count++;
      int64_t down_ms = (s_wifi_disc_time_us > 0) ?
                        (now - s_wifi_disc_time_us) / 1000 : 0;
      ESP_LOGW("net_mon", "WiFi CONNECTED (#%lu, down %lldms)",
               (unsigned long)s_wifi_conn_count, (long long)down_ms);
      snapclient_fr_wifi((uint16_t)id, 1);
    }
  } else if (base == IP_EVENT) {
    if (id == IP_EVENT_STA_GOT_IP) {
      s_ip_got_time_us = now;
      ESP_LOGW("net_mon", "IP acquired");
    } else if (id == IP_EVENT_STA_LOST_IP) {
      s_ip_lost_time_us = now;
      ESP_LOGW("net_mon", "IP LOST");
    }
  }
}

namespace {

static const char *FR_TAG = "snapclient_fr";

enum class FrEvt : uint8_t {
  PLAYER_WR,
  TCP_NOTIFY,
  MUTE,
  NET_MON,
  WIFI_EVT,
  HTTPD,
};

struct FrEntry {
  int64_t  ts_us;
  FrEvt    evt;
  uint8_t  core;
  uint16_t arg16;
  int32_t  a;
  int32_t  b;
  int32_t  c;
};

static constexpr int FR_RING_SZ = 64;
static FrEntry      s_fr_ring[FR_RING_SZ];
static int          s_fr_head = 0;
static bool         s_fr_full = false;
static portMUX_TYPE s_fr_mux = portMUX_INITIALIZER_UNLOCKED;

static FrEntry      s_fr_snap[FR_RING_SZ];

static int  s_fr_q_max = 0;
static bool s_fr_armed = false;
static bool s_fr_dumped = false;
static constexpr int FR_Q_ARM  = 200;
static constexpr int FR_Q_TRIG = 75;

static inline void fr_record(FrEvt e, uint16_t arg16 = 0,
                             int32_t a = 0, int32_t b = 0, int32_t c = 0) {
  portENTER_CRITICAL(&s_fr_mux);
  FrEntry &en = s_fr_ring[s_fr_head];
  en.ts_us = esp_timer_get_time();
  en.evt   = e;
  en.core  = (uint8_t)xPortGetCoreID();
  en.arg16 = arg16;
  en.a     = a;
  en.b     = b;
  en.c     = c;
  s_fr_head = (s_fr_head + 1) % FR_RING_SZ;
  if (s_fr_head == 0) s_fr_full = true;
  portEXIT_CRITICAL(&s_fr_mux);
}

static void fr_dump(const char *reason) {
  int count, start;
  portENTER_CRITICAL(&s_fr_mux);
  count = s_fr_full ? FR_RING_SZ : s_fr_head;
  start = s_fr_full ? s_fr_head : 0;
  for (int i = 0; i < count; i++) {
    s_fr_snap[i] = s_fr_ring[(start + i) % FR_RING_SZ];
  }
  portEXIT_CRITICAL(&s_fr_mux);
  if (count == 0) return;

  int64_t t0 = s_fr_snap[0].ts_us;
  ESP_LOGW(FR_TAG, "===== FLIGHT RECORDER DUMP (%s) %d entries q_max=%d =====",
           reason, count, s_fr_q_max);
  for (int i = 0; i < count; i++) {
    const FrEntry &e = s_fr_snap[i];
    long rel_ms = (long)((e.ts_us - t0) / 1000);
    switch (e.evt) {
      case FrEvt::PLAYER_WR:
        ESP_LOGW(FR_TAG, "[%+6ldms c%u] WR     q=%u ins=%ld age=%ldus sMed=%ldus",
                 rel_ms, e.core, (unsigned)e.arg16, (long)e.a, (long)e.b, (long)e.c);
        break;
      case FrEvt::TCP_NOTIFY:
        ESP_LOGW(FR_TAG, "[%+6ldms c%u] TCPNTF bytes=%u dt=%ldms",
                 rel_ms, e.core, (unsigned)e.arg16, (long)e.a);
        break;
      case FrEvt::MUTE: {
        char tag4[5] = {0};
        memcpy(tag4, &e.a, 4);
        ESP_LOGW(FR_TAG, "[%+6ldms c%u] MUTE   %s caller=%.4s",
                 rel_ms, e.core, e.arg16 ? "ON " : "OFF", tag4);
        break;
      }
      case FrEvt::NET_MON:
        ESP_LOGW(FR_TAG, "[%+6ldms c%u] NETMON heap=%ld rssi=%ld disc=%ld pcb_tw=%u",
                 rel_ms, e.core, (long)e.a, (long)e.b, (long)e.c, (unsigned)e.arg16);
        break;
      case FrEvt::WIFI_EVT:
        ESP_LOGW(FR_TAG, "[%+6ldms c%u] WIFI   evt=%u connected=%ld",
                 rel_ms, e.core, (unsigned)e.arg16, (long)e.a);
        break;
      case FrEvt::HTTPD:
        ESP_LOGW(FR_TAG, "[%+6ldms c%u] HTTPD  %s dur=%ldms",
                 rel_ms, e.core, e.arg16 ? "POST" : "GET ", (long)e.a);
        break;
      default:
        ESP_LOGW(FR_TAG, "[%+6ldms c%u] ???    arg16=%u a=%ld",
                 rel_ms, e.core, (unsigned)e.arg16, (long)e.a);
        break;
    }
  }
  ESP_LOGW(FR_TAG, "===== END FLIGHT RECORDER DUMP =====");
}

static inline void fr_check_q(int q) {
  if (q > s_fr_q_max) s_fr_q_max = q;
  if (q >= FR_Q_ARM) {
    s_fr_armed = true;
    s_fr_dumped = false;
  }
  if (s_fr_armed && !s_fr_dumped && q < FR_Q_TRIG) {
    s_fr_dumped = true;
    fr_dump("q-stall");
  }
}

}

extern "C" void snapclient_fr_mute(int on, const char *caller) {
  uint32_t tag4 = 0;
  if (caller) {
    for (int i = 0; i < 4 && caller[i]; i++)
      ((char *)&tag4)[i] = caller[i];
  }
  fr_record(FrEvt::MUTE, on ? 1 : 0, (int32_t)tag4);
}

extern "C" void snapclient_fr_wifi(uint16_t evt_id, int connected) {
  fr_record(FrEvt::WIFI_EVT, evt_id, connected);
}

extern "C" void snapclient_fr_httpd(int is_post, int dur_ms) {
  fr_record(FrEvt::HTTPD, is_post ? 1 : 0, dur_ms);
}

namespace esphome {
namespace snapclient {

struct SnapMetaUpdate {
  char title[96];
  char artist[64];
  char album[64];
  char album_art_url[256];
  float duration_s;
  float position_s;
  bool  reset_position;
  bool  is_clear;
};

static constexpr const char *NVS_NAMESPACE = "snapclient";
static constexpr const char *NVS_KEY_VOLUME = "snap_vol";

static float nvs_load_float(const char *key, float default_val) {
  nvs_handle_t handle;
  if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle) != ESP_OK)
    return default_val;
  uint32_t raw = 0;
  float out = default_val;
  if (nvs_get_u32(handle, key, &raw) == ESP_OK)
    memcpy(&out, &raw, sizeof(float));
  nvs_close(handle);
  return out;
}

static void nvs_save_float(const char *key, float value) {
  nvs_handle_t handle;
  if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK)
    return;
  uint32_t raw = 0;
  memcpy(&raw, &value, sizeof(float));
  nvs_set_u32(handle, key, raw);
  nvs_commit(handle);
  nvs_close(handle);
}

static bool s_write_cb_reset_pending = false;
static void snapclient_write_cb_reset(void) { s_write_cb_reset_pending = true; }

static void snapclient_write_cb(const void *buf, size_t bytes, size_t *written, void *ctx) {
  auto *self = static_cast<SnapClientComponent *>(ctx);
  *written = 0;
  if (self->source_ == nullptr) return;

  static uint32_t wr_cnt = 0;
  static uint32_t wr_zero = 0;
  static uint32_t wr_consec_zero = 0;
  static bool mixer_was_down = false;
  static uint32_t skip_probe_cnt = 0;

  if (s_write_cb_reset_pending) {
    wr_cnt = 0;
    wr_zero = 0;
    wr_consec_zero = 0;
    mixer_was_down = false;
    skip_probe_cnt = 0;
    s_write_cb_reset_pending = false;
  }

  if (mixer_was_down) {
    wr_zero++;
    wr_consec_zero++;
    if (++skip_probe_cnt < 10) {
      goto telemetry;
    }
    skip_probe_cnt = 0;

  }

  {

    static uint8_t pcm_buf[4096];
    size_t n = std::min(bytes, sizeof(pcm_buf));
    memcpy(pcm_buf, buf, n);

#ifdef USE_SHARED_AUDIO_EQ
    if (n >= 4) {
      eq_process_with_volume((int16_t*)pcm_buf, n / 4, EQ_SOURCE_SNAPCLIENT);
    }
#endif

    int64_t wr_start = esp_timer_get_time();
    *written = self->source_->play(pcm_buf, n, pdMS_TO_TICKS(100));
    int64_t wr_dur_ms = (esp_timer_get_time() - wr_start) / 1000;
    if (wr_dur_ms > 200) {
      static uint32_t last_slow_log_ms = 0;
      uint32_t now_ms = (uint32_t)(esp_timer_get_time() / 1000);
      if (now_ms - last_slow_log_ms > 5000) {
        last_slow_log_ms = now_ms;
        ESP_LOGW(TAG, "WRITE_SLOW: play() blocked %lldms written=%u/%u",
                 wr_dur_ms, (unsigned)*written, (unsigned)n);
      }
    }
  }

  if (*written == 0) {
    wr_zero++;
    wr_consec_zero++;
    if (wr_consec_zero >= 3) {
      mixer_was_down = true;
    }
  } else {
    if (mixer_was_down) {
      mixer_was_down = false;
      skip_probe_cnt = 0;
      player_request_resync();
      ESP_LOGW(TAG, "write_cb: mixer recovered after %lu zeros — resync requested",
               (unsigned long)wr_consec_zero);
    }
    wr_consec_zero = 0;
  }

telemetry:
  if (++wr_cnt >= 200) {
    player_telemetry_t t;
    player_get_telemetry(&t);
    ESP_LOGD(TAG, "wr: %u/%u z=%lu | age=%lldus sMed=%lldus q=%d ins=%lld",
             (unsigned)*written, (unsigned)bytes, (unsigned long)wr_zero,
             t.age_us, t.short_median_us, t.queue_depth, t.inserted_samples);

    fr_record(FrEvt::PLAYER_WR,
              (uint16_t)(t.queue_depth < 0 ? 0 : (t.queue_depth > 65535 ? 65535 : t.queue_depth)),
              (int32_t)t.inserted_samples,
              (int32_t)t.age_us,
              (int32_t)t.short_median_us);
    fr_check_q((int)t.queue_depth);

    if (self->has_any_tele_) {
      self->tele_age_us_          = t.age_us;
      self->tele_short_median_us_ = t.short_median_us;
      self->tele_mini_median_us_  = t.mini_median_us;
      self->tele_queue_depth_     = t.queue_depth;
      self->tele_heap_            = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
      self->tele_psram_           = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
      uint32_t d_recv, d_ok, d_drop;
      decoder_get_counters(&d_recv, &d_ok, &d_drop);
      self->tele_recv_            = d_recv;
      self->tele_decoded_         = d_ok;
      self->tele_drop_            = d_drop;
      self->tele_fresh_           = true;
    }

    wr_cnt = 0;
    wr_zero = 0;
  }
}

void SnapClientComponent::setup() {
  ESP_LOGV(TAG, "setup() executing");

  this->audio_q_hdl_ = xQueueCreate(1, sizeof(audioDACdata_t));

  if (this->mute_pin_ != nullptr) {
    this->mute_pin_->setup();
    this->mute_pin_->digital_write(false);
  }

  this->volume_ = nvs_load_float(NVS_KEY_VOLUME, 0.0f);
  ESP_LOGD(TAG, "Loaded volume from NVS: %.2f", this->volume_);
  this->boot_volume_ = (this->volume_ > 0.0f) ? this->volume_ : 0.5f;

  this->set_volume_(this->boot_volume_, false, false);

  this->http_rpc_queue_ = xQueueCreate(1, sizeof(rpc_request_t));

  this->meta_q_hdl_ = xQueueCreate(1, sizeof(SnapMetaUpdate));

  if (this->track_name_sensor_) this->track_name_sensor_->publish_state("");
  if (this->artist_sensor_)     this->artist_sensor_->publish_state("");
  if (this->album_sensor_)      this->album_sensor_->publish_state("");
  if (this->album_art_url_sensor_) this->album_art_url_sensor_->publish_state("");

  init_snapcast(this->audio_q_hdl_, this->name_.c_str(), this->host_.c_str(), this->port_);
  init_player(snapclient_write_cb, this);

#ifdef CONFIG_WEB_PORT
#ifdef USE_ETHERNET
  init_http_server_task("ETH_DEF");
#endif
#ifdef USE_WIFI
  init_http_server_task("WIFI_STA_DEF");
#endif
#endif

#if CONFIG_USE_DSP_PROCESSOR
  dsp_processor_init();
#endif

  this->network_initialized_ = false;
  this->state = media_player::MEDIA_PLAYER_STATE_IDLE;

#ifdef USE_WIFI

  if (wifi::global_wifi_component)
    wifi::global_wifi_component->set_post_connect_roaming(false);
  ESP_LOGD(TAG, "WiFi roam-scan suppressed (snapclient active)");
#endif

  ESP_LOGD(TAG, "Create Task");
  this->snapclient_start();
}

void SnapClientComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Snapclient Media Player:");
  ESP_LOGCONFIG(TAG, "  Host: %s", this->host_.c_str());
  ESP_LOGCONFIG(TAG, "  Port: %d", this->port_);
#ifdef USE_AUDIO_DAC
  ESP_LOGCONFIG(TAG, "  Audio DAC: %s", this->audio_dac_ != nullptr ? "configured" : "None");
#else
  ESP_LOGCONFIG(TAG, "  Audio DAC: None");
#endif
  ESP_LOGCONFIG(TAG, "  dB range: %.1f to %.1f", this->snapcast_min_db_, this->snapcast_max_db_);
  ESP_LOGCONFIG(TAG, "  Volume range: %.0f%% to %.0f%%", this->min_volume_pct_ * 100, this->max_volume_pct_ * 100);
}

void SnapClientComponent::snapclient_stop_begin() {
  if (!this->snapclient_running_) return;
  ESP_LOGI(TAG, "snapclient_stop_begin: signaling stop (heap=%u)",
           (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));

  this->stop_requested_ = true;
  decoder_request_stop();
  player_request_stop();

  if (this->tcp_notify_sock_ >= 0) {
    lwip_close(this->tcp_notify_sock_);
    this->tcp_notify_sock_ = -1;
  }
}

bool SnapClientComponent::snapclient_tasks_stopped() {
  return !this->http_get_task_handle_ &&
         !this->http_rpc_task_handle_ &&
         !this->tcp_notify_task_handle_ &&
         !this->net_monitor_task_handle_ &&
         !is_player_running();
}

void SnapClientComponent::snapclient_stop_finalize() {
  deinit_player();

  if (this->http_get_task_handle_) {
    vTaskDelete(this->http_get_task_handle_);
    this->http_get_task_handle_ = nullptr;
  }
  if (this->http_rpc_task_handle_) {
    vTaskDelete(this->http_rpc_task_handle_);
    this->http_rpc_task_handle_ = nullptr;
  }
  if (this->tcp_notify_task_handle_) {
    vTaskDelete(this->tcp_notify_task_handle_);
    this->tcp_notify_task_handle_ = nullptr;
  }
  if (this->net_monitor_task_handle_) {
    vTaskDelete(this->net_monitor_task_handle_);
    this->net_monitor_task_handle_ = nullptr;
  }

  this->snapclient_running_ = false;
  ESP_LOGI(TAG, "snapclient_stop_finalize: DONE (heap=%u)",
           (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
}

void SnapClientComponent::snapclient_start() {
  if (this->snapclient_running_) return;
  ESP_LOGI(TAG, "snapclient_start: BEGIN");

  this->stop_requested_ = false;
  decoder_clear_stop();
  snapclient_write_cb_reset();
  init_player(snapclient_write_cb, this);

  xTaskCreatePinnedToCore(&http_get_task, "http", 4 * 1024, this,
                          HTTP_TASK_PRIORITY, &this->http_get_task_handle_,
                          HTTP_TASK_CORE_ID);

  if (!this->host_.empty()) {
    xTaskCreatePinnedToCore(http_rpc_task_, "snap_rpc", 4096, this, 1,
                            &this->http_rpc_task_handle_, 0);
    xTaskCreatePinnedToCore(tcp_notify_task_, "snap_tcp_notify", 6144, this, 1,
                            &this->tcp_notify_task_handle_, 0);
    xTaskCreatePinnedToCore(net_monitor_task_, "net_mon", 3072, this, 1,
                            &this->net_monitor_task_handle_, 0);
  }

  this->snapclient_running_ = true;
  ESP_LOGI(TAG, "snapclient_start: DONE");
}

float SnapClientComponent::map_volume_to_eq_gain_(float vol) {
  const float scaled = this->min_volume_pct_ +
                       vol * (this->max_volume_pct_ - this->min_volume_pct_);
  const float db = this->snapcast_min_db_ +
                   scaled * (this->snapcast_max_db_ - this->snapcast_min_db_);
  const float linear = powf(10.0f, db / 20.0f);
  const float linear_max = powf(10.0f, this->snapcast_max_db_ / 20.0f);
  return (linear_max > 0.0f) ? (linear / linear_max) : 0.0f;
}

void SnapClientComponent::loop() {
  if (!this->network_initialized_ && network::is_connected()) {
    this->network_initialized_ = true;
    this->state = media_player::MEDIA_PLAYER_STATE_IDLE;
    this->publish_state();

    if (!this->host_.empty()) {
      this->boot_push_at_ms_ = millis() + 6000;
    }
  }

  {
    using S = GateState;
    bool cspot_on = esphome::snapspot::cspot_active.load(std::memory_order_acquire);

    switch (this->gate_state_) {
    case S::IDLE:
      if (cspot_on) {
        ESP_LOGI(TAG, "[gate] client active -> signaling snapclient stop");
        this->snapclient_stop_begin();
        this->gate_ts_ = millis();
        this->gate_state_ = S::STOPPING;
      }
      break;

    case S::STOPPING:
      if (this->snapclient_tasks_stopped()) {
        ESP_LOGI(TAG, "[gate] snapclient tasks stopped -> finalizing stop");
        this->snapclient_stop_finalize();
        esphome::snapspot::audio_path_free.store(true, std::memory_order_release);
        this->gate_state_ = S::CLIENT_ACTIVE;
      } else if (millis() - this->gate_ts_ > 10000) {
        ESP_LOGE(TAG, "[gate] timeout waiting for snapclient tasks -> force finalize");
        this->snapclient_stop_finalize();
        esphome::snapspot::audio_path_free.store(true, std::memory_order_release);
        this->gate_state_ = S::CLIENT_ACTIVE;
      }
      break;

    case S::CLIENT_ACTIVE:
      if (esphome::snapspot::cspot_shutdown_pending.load(std::memory_order_acquire)) {
        ESP_LOGI(TAG, "[gate] shutdown detected -> closing client socket");
        if (esphome::snapspot::cspot_callbacks.close_connection)
          esphome::snapspot::cspot_callbacks.close_connection(esphome::snapspot::cspot_callbacks.ctx);
        this->gate_ts_ = millis();
        this->gate_state_ = S::DISCONNECTING;
      } else if (!cspot_on) {
        if (esphome::snapspot::cspot_fully_stopped.load(std::memory_order_acquire)) {
          ESP_LOGI(TAG, "[gate] client stopped -> starting snapclient");
          esphome::snapspot::audio_path_free.store(false, std::memory_order_release);
          this->snapclient_start();
          this->gate_state_ = S::IDLE;
        }
      }
      break;

    case S::DISCONNECTING: {
      bool re_auth = !esphome::snapspot::cspot_shutdown_pending.load(std::memory_order_acquire) && cspot_on;
      if (re_auth) {
        ESP_LOGI(TAG, "[gate] re-auth during shutdown -> back to active");
        this->gate_ts_ = millis();
        this->gate_state_ = S::CLIENT_ACTIVE;
      } else {
        bool tasks_done = esphome::snapspot::cspot_callbacks.are_tasks_exited &&
                          esphome::snapspot::cspot_callbacks.are_tasks_exited(esphome::snapspot::cspot_callbacks.ctx);
        if (tasks_done) {
          ESP_LOGI(TAG, "[gate] tasks exited -> finalizing cleanup");
          if (esphome::snapspot::cspot_callbacks.finalize_cleanup)
            esphome::snapspot::cspot_callbacks.finalize_cleanup(esphome::snapspot::cspot_callbacks.ctx);
          this->gate_state_ = S::FINALIZING;
        } else if (millis() - this->gate_ts_ > 5000) {
          ESP_LOGE(TAG, "[gate] timeout waiting for tasks -> force cleanup");
          if (esphome::snapspot::cspot_callbacks.force_cleanup)
            esphome::snapspot::cspot_callbacks.force_cleanup(esphome::snapspot::cspot_callbacks.ctx);
          this->gate_state_ = S::FINALIZING;
        }
      }
      break;
    }

    case S::FINALIZING:
      if (esphome::snapspot::cspot_fully_stopped.load(std::memory_order_acquire)) {
        ESP_LOGI(TAG, "[gate] client fully stopped -> starting snapclient (heap=%u)",
                 (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
        esphome::snapspot::audio_path_free.store(false, std::memory_order_release);
        this->snapclient_start();
        this->gate_state_ = S::IDLE;
      } else if (millis() - this->gate_ts_ > 15000) {
        ESP_LOGE(TAG, "[gate] FINALIZING timeout (15s) -> force IDLE, restarting snapclient");
        esphome::snapspot::cspot_fully_stopped.store(true, std::memory_order_release);
        esphome::snapspot::cspot_active.store(false, std::memory_order_release);
        esphome::snapspot::cspot_shutdown_pending.store(false, std::memory_order_release);
        esphome::snapspot::audio_path_free.store(false, std::memory_order_release);
        this->snapclient_start();
        this->gate_state_ = S::IDLE;
      }
      break;
    }
  }

  if (this->boot_push_at_ms_ && millis() >= this->boot_push_at_ms_) {
    this->boot_push_at_ms_ = 0;
    this->mute_state_ = false;
    ESP_LOGD(TAG, "boot_push: restoring volume %.2f to HA and Snapserver", this->boot_volume_);
    this->set_volume_(this->boot_volume_, true, true);
  }

  audioDACdata_t dac_data;
  static audioDACdata_t dac_data_old = {
      .mute = true,
      .volume = 100,
  };
  if (xQueueReceive(this->audio_q_hdl_, &dac_data, 0) == pdTRUE) {

    if (dac_data.mute != dac_data_old.mute) {
      if (this->mute_pin_ != nullptr) {
        this->mute_pin_->digital_write(!dac_data.mute);
      }
#ifdef USE_SHARED_AUDIO_EQ
      if (dac_data.mute) {
        eq_set_source_gain(EQ_SOURCE_SNAPCLIENT, 0.0f);
      } else {
        float gain = this->map_volume_to_eq_gain_(this->volume_);
        eq_set_source_gain(EQ_SOURCE_SNAPCLIENT, gain);
      }
#endif
      this->mute_state_ = dac_data.mute;
      this->publish_state();
      ESP_LOGD(TAG, "%s", dac_data.mute ? "Mute" : "Unmute");
    }

    if (dac_data.volume != dac_data_old.volume) {
      uint32_t now = millis();
      if (this->last_ha_volume_ms_ != 0 && now - this->last_ha_volume_ms_ < 2000) {
        ESP_LOGD(TAG, "server echo suppressed (debounce %ums)",
                 (unsigned)(now - this->last_ha_volume_ms_));
      } else {
        this->last_server_volume_ms_ = now;
        float vol = (float)dac_data.volume / 100.0f;
        this->volume_ = vol;
        this->volume = vol;

        float gain = this->map_volume_to_eq_gain_(vol);
#ifdef USE_SHARED_AUDIO_EQ
        eq_set_source_gain(EQ_SOURCE_SNAPCLIENT, gain);
        ESP_LOGD(TAG, "server volume -> %.2f (eq gain %.3f)", vol, gain);
#endif
        nvs_save_float(NVS_KEY_VOLUME, vol);
        this->publish_state();
      }
    }
    dac_data_old = dac_data;
  }

  SnapMetaUpdate meta_upd;
  if (this->meta_q_hdl_ &&
      xQueueReceive(this->meta_q_hdl_, &meta_upd, 0) == pdTRUE) {
    if (meta_upd.is_clear) {
      if (this->track_name_sensor_)    this->track_name_sensor_->publish_state("");
      if (this->artist_sensor_)        this->artist_sensor_->publish_state("");
      if (this->album_sensor_)         this->album_sensor_->publish_state("");
      if (this->album_art_url_sensor_) this->album_art_url_sensor_->publish_state("");
      if (this->duration_sensor_)  this->duration_sensor_->publish_state(0.0f);
      if (this->position_sensor_)  this->position_sensor_->publish_state(0.0f);
      this->pos_current_s_    = 0.0f;
      this->pos_last_tick_ms_ = 0;
      ESP_LOGD(TAG, "meta queue: is_clear — sensors cleared");
    } else {
      if (this->track_name_sensor_)    this->track_name_sensor_->publish_state(meta_upd.title);
      if (this->artist_sensor_)        this->artist_sensor_->publish_state(meta_upd.artist);
      if (this->album_sensor_)         this->album_sensor_->publish_state(meta_upd.album);
      if (this->album_art_url_sensor_) this->album_art_url_sensor_->publish_state(meta_upd.album_art_url);
      if (this->duration_sensor_)
        this->duration_sensor_->publish_state(meta_upd.duration_s >= 0.0f ? meta_upd.duration_s : 0.0f);
      if (meta_upd.reset_position) {
        this->pos_current_s_    = 0.0f;
        this->pos_last_tick_ms_ = millis();
        if (this->position_sensor_) this->position_sensor_->publish_state(0.0f);
      }
      ESP_LOGV(TAG, "meta: title='%s' artist='%s' reset_pos=%d",
               meta_upd.title, meta_upd.artist, (int)meta_upd.reset_position);
    }
  }

  if (this->position_sensor_ &&
      this->state == media_player::MEDIA_PLAYER_STATE_PLAYING &&
      this->pos_last_tick_ms_ != 0) {
    uint32_t now_pos = millis();
    if (now_pos - this->pos_last_tick_ms_ >= 1000) {
      float old_pos = this->pos_current_s_;
      this->pos_current_s_ += 1.0f;
      this->pos_last_tick_ms_ = now_pos;
      if ((int)(this->pos_current_s_) / 5 > (int)(old_pos) / 5) {
        this->position_sensor_->publish_state(this->pos_current_s_);
      }
    }
  }

  if (this->has_any_tele_ && this->tele_fresh_) {
    this->tele_fresh_ = false;
    if (this->sync_age_sensor_)        this->sync_age_sensor_->publish_state((float)this->tele_age_us_);
    if (this->short_median_sensor_)    this->short_median_sensor_->publish_state((float)this->tele_short_median_us_);
    if (this->mini_median_sensor_)     this->mini_median_sensor_->publish_state((float)this->tele_mini_median_us_);
    if (this->queue_depth_sensor_)     this->queue_depth_sensor_->publish_state((float)this->tele_queue_depth_);
    if (this->heap_free_sensor_)       this->heap_free_sensor_->publish_state((float)this->tele_heap_);
    if (this->psram_free_sensor_)      this->psram_free_sensor_->publish_state((float)this->tele_psram_);
    if (this->decoder_recv_sensor_)    this->decoder_recv_sensor_->publish_state((float)this->tele_recv_);
    if (this->decoder_decoded_sensor_) this->decoder_decoded_sensor_->publish_state((float)this->tele_decoded_);
    if (this->decoder_drop_sensor_)    this->decoder_drop_sensor_->publish_state((float)this->tele_drop_);
    if (this->drift_sensor_) {
      double drift = player_get_latency_drift();
      if (std::isfinite(drift))
        this->drift_sensor_->publish_state((float)(drift * 1000000.0));
    }
  }

#ifdef USE_WIFI
  if (this->state != this->last_state_) {
    if (this->state == media_player::MEDIA_PLAYER_STATE_PLAYING) {
      if (wifi::global_wifi_component)
        wifi::global_wifi_component->set_post_connect_roaming(false);
      ESP_LOGD(TAG, "WiFi roam-scan suppressed (PLAYING)");
    } else if (this->last_state_ == media_player::MEDIA_PLAYER_STATE_PLAYING) {
      if (wifi::global_wifi_component)
        wifi::global_wifi_component->set_post_connect_roaming(true);
      ESP_LOGD(TAG, "WiFi roam-scan restored (not PLAYING)");
    }
    this->last_state_ = this->state;
  }
#endif
}

void SnapClientComponent::control(const media_player::MediaPlayerCall &call) {
  if (this->state == media_player::MEDIA_PLAYER_STATE_OFF ||
      this->state == media_player::MEDIA_PLAYER_STATE_NONE) {
    ESP_LOGW(TAG, "Player not ready. Ignoring control command.");
    return;
  }

  if (call.get_volume().has_value()) {
    this->set_volume_(call.get_volume().value());
  }

  if (call.get_command().has_value()) {
    switch (call.get_command().value()) {
      case media_player::MEDIA_PLAYER_COMMAND_MUTE:
        this->set_mute_(true);
        break;
      case media_player::MEDIA_PLAYER_COMMAND_UNMUTE:
        this->set_mute_(false);
        break;
      default:
        break;
    }
  }
}

void SnapClientComponent::set_mute_(bool mute) {
  this->mute_state_ = mute;
#ifdef USE_SHARED_AUDIO_EQ
  if (mute) {
    eq_set_source_gain(EQ_SOURCE_SNAPCLIENT, 0.0f);
  } else {
    float gain = this->map_volume_to_eq_gain_(this->volume_);
    eq_set_source_gain(EQ_SOURCE_SNAPCLIENT, gain);
  }
#endif
  if (this->mute_pin_ != nullptr)
    this->mute_pin_->digital_write(!mute);
  this->push_mute_to_server_(mute);
  this->publish_state();
  ESP_LOGD(TAG, "set_mute_: %s", mute ? "muted" : "unmuted");
}

void SnapClientComponent::set_volume_(float volume, bool publish, bool push_server) {
  volume = std::max(0.0f, std::min(1.0f, volume));
  this->volume_ = volume;
  this->volume = volume;

  float gain = this->map_volume_to_eq_gain_(volume);

#ifdef USE_SHARED_AUDIO_EQ
  eq_set_source_gain(EQ_SOURCE_SNAPCLIENT, gain);
  ESP_LOGD(TAG, "set_volume_: %.2f (eq gain %.3f)", volume, gain);
#else
  ESP_LOGD(TAG, "set_volume_: %.2f (no EQ, gain %.3f)", volume, gain);
#endif

  nvs_save_float(NVS_KEY_VOLUME, volume);

  if (push_server) {
    this->push_volume_to_server_(volume);
  }

  if (publish) {
    this->publish_state();
  }
}

media_player::MediaPlayerTraits SnapClientComponent::get_traits() {
  auto traits = media_player::MediaPlayerTraits();
  traits.set_supports_pause(true);
  return traits;
}

void SnapClientComponent::push_volume_to_server_(float volume) {
  if (this->host_.empty()) return;
  uint32_t now = millis();
  if (now - this->last_server_volume_ms_ < 2000) {
    ESP_LOGD(TAG, "push_volume_to_server_: debounced (server-echo guard)");
    return;
  }
  this->last_ha_volume_ms_ = now;
  if (!this->http_rpc_queue_) return;
  rpc_request_t req{(int)(volume * 100.0f), this->mute_state_};
  xQueueOverwrite(this->http_rpc_queue_, &req);
  ESP_LOGD(TAG, "push_volume_to_server_: queued percent=%d", req.percent);
}

void SnapClientComponent::push_mute_to_server_(bool mute) {
  if (this->host_.empty()) return;
  uint32_t now = millis();
  if (now - this->last_server_volume_ms_ < 2000) return;
  this->last_ha_volume_ms_ = now;
  if (!this->http_rpc_queue_) return;
  rpc_request_t req{(int)(this->volume_ * 100.0f), mute};
  xQueueOverwrite(this->http_rpc_queue_, &req);
  ESP_LOGD(TAG, "push_mute_to_server_: queued muted=%d", (int)mute);
}

static bool send_jsonrpc(const char *host, const char *body) {
  int64_t t0 = esp_timer_get_time();
  struct addrinfo hints{};
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  struct addrinfo *res = nullptr;
  if (getaddrinfo(host, "1780", &hints, &res) != 0 || !res) {
    int64_t dns_ms = (esp_timer_get_time() - t0) / 1000;
    ESP_LOGW(TAG, "jsonrpc: DNS failed after %lldms", (long long)dns_ms);
    return false;
  }
  int64_t dns_ms = (esp_timer_get_time() - t0) / 1000;

  int sock = socket(res->ai_family, res->ai_socktype, 0);
  if (sock < 0) { freeaddrinfo(res); return false; }

  struct timeval tv{2, 0};
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  int64_t conn_t0 = esp_timer_get_time();
  bool ok = false;
  if (connect(sock, res->ai_addr, res->ai_addrlen) == 0) {
    int64_t conn_ms = (esp_timer_get_time() - conn_t0) / 1000;
    char header[256];
    int hlen = snprintf(header, sizeof(header),
                        "POST /jsonrpc HTTP/1.1\r\n"
                        "Host: %s:1780\r\n"
                        "Content-Type: application/json\r\n"
                        "Content-Length: %d\r\n"
                        "Connection: close\r\n\r\n",
                        host, (int)strlen(body));
    send(sock, header, hlen, 0);
    send(sock, body, strlen(body), 0);
    char resp[512];
    int total = 0;
    int rn;
    int64_t recv_t0 = esp_timer_get_time();
    while (total < (int)sizeof(resp) - 1) {
      rn = recv(sock, resp + total, sizeof(resp) - 1 - total, 0);
      if (rn <= 0) break;
      total += rn;
    }
    int64_t recv_ms = (esp_timer_get_time() - recv_t0) / 1000;
    if (total > 0) {
      resp[total] = '\0';
      ok = (strstr(resp, "\"error\"") == nullptr);
      if (!ok) ESP_LOGW(TAG, "jsonrpc error: %s", resp);
    }
    int64_t total_ms = (esp_timer_get_time() - t0) / 1000;
    if (total_ms > 100) {
      ESP_LOGW(TAG, "jsonrpc: slow dns=%lldms conn=%lldms recv=%lldms total=%lldms",
               (long long)dns_ms, (long long)conn_ms, (long long)recv_ms, (long long)total_ms);
    }
  } else {
    int64_t conn_ms = (esp_timer_get_time() - conn_t0) / 1000;
    ESP_LOGW(TAG, "jsonrpc: connect failed after %lldms errno=%d", (long long)conn_ms, errno);
  }
  close(sock);
  freeaddrinfo(res);
  return ok;
}

void SnapClientComponent::http_rpc_task_(void *pvParam) {
  auto *self = static_cast<SnapClientComponent *>(pvParam);

  uint8_t mac[6] = {};
  esp_wifi_get_mac(WIFI_IF_STA, mac);
  char mac_str[18];
  snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  const char *host = self->host_.c_str();
  ESP_LOGD(TAG, "http_rpc_task_: ready  host=%s:1780  client-id=%s", host, mac_str);

  while (!self->stop_requested_) {
    rpc_request_t req;
    if (xQueueReceive(self->http_rpc_queue_, &req, pdMS_TO_TICKS(500)) != pdTRUE)
      continue;
      char body[256];
      snprintf(body, sizeof(body),
               "{\"id\":1,\"jsonrpc\":\"2.0\",\"method\":\"Client.SetVolume\","
               "\"params\":{\"id\":\"%s\","
               "\"volume\":{\"percent\":%d,\"muted\":%s}}}",
               mac_str, req.percent, req.muted ? "true" : "false");
      ESP_LOGD(TAG, "http_rpc_task_: POST %s", body);
      bool ok = send_jsonrpc(host, body);
      ESP_LOGD(TAG, "http_rpc_task_: %s", ok ? "OK" : "failed");
  }
  self->http_rpc_task_handle_ = nullptr;
  vTaskDelete(nullptr);
}

static void copy_json_string(cJSON *object, const char *key, char *out, size_t out_len) {
  cJSON *value = cJSON_GetObjectItemCaseSensitive(object, key);
  if (cJSON_IsString(value) && value->valuestring) {
    snprintf(out, out_len, "%s", value->valuestring);
  }
}

static void copy_json_string_or_first_array(cJSON *object, const char *key,
                                            char *out, size_t out_len) {
  cJSON *value = cJSON_GetObjectItemCaseSensitive(object, key);
  if (cJSON_IsString(value) && value->valuestring) {
    snprintf(out, out_len, "%s", value->valuestring);
    return;
  }
  if (cJSON_IsArray(value) && cJSON_GetArraySize(value) > 0) {
    cJSON *first = cJSON_GetArrayItem(value, 0);
    if (cJSON_IsString(first) && first->valuestring) {
      snprintf(out, out_len, "%s", first->valuestring);
    }
  }
}

static float copy_json_number(cJSON *object, const char *key, float default_val) {
  cJSON *value = cJSON_GetObjectItemCaseSensitive(object, key);
  if (cJSON_IsNumber(value)) return (float)value->valuedouble;
  return default_val;
}

void SnapClientComponent::tcp_notify_task_(void *pvParam) {
  auto *self = static_cast<SnapClientComponent *>(pvParam);
  const char *host = self->host_.c_str();
  int sock = -1;
  bool resumed_from_idle = false;

  const size_t LINE_BUF_SIZE = 4096;
  char *line_buf = static_cast<char *>(malloc(LINE_BUF_SIZE));
  if (!line_buf) {
    ESP_LOGE(TAG, "tcp_notify_task_: OOM");
    vTaskDelete(nullptr);
    return;
  }

  const size_t RD_BUF_SIZE = 512;
  char *rd_buf = static_cast<char *>(malloc(RD_BUF_SIZE));
  if (!rd_buf) {
    ESP_LOGE(TAG, "tcp_notify_task_: OOM rd_buf");
    free(line_buf);
    vTaskDelete(nullptr);
    return;
  }
  int rd_pos = 0, rd_len = 0;

  while (!self->stop_requested_) {

    if (sock == -1) {
      struct addrinfo hints{};
      hints.ai_family   = AF_INET;
      hints.ai_socktype = SOCK_STREAM;
      struct addrinfo *res = nullptr;
      int64_t dns_t0 = esp_timer_get_time();
      int err = lwip_getaddrinfo(host, "1705", &hints, &res);
      int64_t dns_ms = (esp_timer_get_time() - dns_t0) / 1000;
      if (err != 0 || res == nullptr) {
        ESP_LOGW(TAG, "tcp_notify: DNS failed for %s: %d (%lldms)", host, err, (long long)dns_ms);
        vTaskDelay(pdMS_TO_TICKS(3000));
        continue;
      }
      if (dns_ms > 50) {
        ESP_LOGW(TAG, "tcp_notify: DNS took %lldms", (long long)dns_ms);
      }
      int fd = lwip_socket(res->ai_family, res->ai_socktype, res->ai_protocol);
      if (fd >= 0) {
        struct timeval tv{ .tv_sec = 3, .tv_usec = 0 };
        lwip_setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        int64_t conn_t0 = esp_timer_get_time();
        if (lwip_connect(fd, res->ai_addr, res->ai_addrlen) == 0) {
          int64_t conn_ms = (esp_timer_get_time() - conn_t0) / 1000;
          if (conn_ms > 50) {
            ESP_LOGW(TAG, "tcp_notify: connect took %lldms", (long long)conn_ms);
          }

          const char req[] = "{\"id\":1,\"jsonrpc\":\"2.0\",\"method\":\"Server.GetStatus\"}\n";
          lwip_send(fd, req, sizeof(req) - 1, 0);
          int64_t boot_t0 = esp_timer_get_time();
          size_t pos = 0;
          bool got_nl = false;
          while (!got_nl) {
            char ch;
            int n = lwip_recv(fd, &ch, 1, MSG_DONTWAIT);
            if (n <= 0) break;
            if (ch == '\n') got_nl = true;
            if (++pos > 8192) break;
          }
          int64_t boot_ms = (esp_timer_get_time() - boot_t0) / 1000;
          if (boot_ms > 50) {
            ESP_LOGW(TAG, "tcp_notify: bootstrap recv %lldms %u bytes", (long long)boot_ms, (unsigned)pos);
          }
          sock = fd;
          self->tcp_notify_sock_ = fd;
          self->tcp_last_lock_ms_ = millis();
          resumed_from_idle = false;
          rd_pos = 0;
          rd_len = 0;
          memset(self->dedup_title_,  0, sizeof(self->dedup_title_));
          memset(self->dedup_artist_, 0, sizeof(self->dedup_artist_));
          ESP_LOGI(TAG, "tcp_notify: connected to %s:1705", host);
        } else {
          int64_t conn_ms = (esp_timer_get_time() - conn_t0) / 1000;
          ESP_LOGW(TAG, "tcp_notify: connect failed errno=%d (%lldms)", errno, (long long)conn_ms);
          lwip_close(fd);
          vTaskDelay(pdMS_TO_TICKS(3000));
        }
      }
      lwip_freeaddrinfo(res);
      if (sock == -1) continue;
    }

    size_t pos = 0;
    bool got_line = false;
    int64_t recv_t0 = esp_timer_get_time();
    while (pos < LINE_BUF_SIZE - 1) {
      if (rd_pos >= rd_len) {
        int n = lwip_recv(sock, rd_buf, RD_BUF_SIZE, 0);
        if (n <= 0) {
          if (!self->stop_requested_) {
            ESP_LOGW(TAG, "tcp_notify: recv %s after %u bytes — reconnecting",
                     n == 0 ? "closed" : "error", (unsigned)pos);
          }
          lwip_close(sock);
          sock = -1;
          self->tcp_notify_sock_ = -1;
          rd_pos = 0;
          rd_len = 0;
          break;
        }
        rd_pos = 0;
        rd_len = n;
      }
      while (rd_pos < rd_len && pos < LINE_BUF_SIZE - 1) {
        char ch = rd_buf[rd_pos++];
        if (ch == '\n') { got_line = true; goto line_done; }
        line_buf[pos++] = ch;
      }
    }
    line_done:
    if (!got_line || sock == -1) continue;
    int64_t recv_us = esp_timer_get_time() - recv_t0;

    fr_record(FrEvt::TCP_NOTIFY,
              (uint16_t)(pos > 65535 ? 65535 : pos),
              (int32_t)(recv_us / 1000));

    if (recv_us > 100000) {
      ESP_LOGW(TAG, "tcp_notify: recv-loop %lldms for %u bytes", (long long)(recv_us / 1000), (unsigned)pos);
    }
    line_buf[pos] = '\0';

    int64_t parse_t0 = esp_timer_get_time();
    cJSON *root = cJSON_Parse(line_buf);
    int64_t parse_us = esp_timer_get_time() - parse_t0;
    if (!root) {
      if (parse_us > 10000) {
        ESP_LOGW(TAG, "tcp_notify: cJSON_Parse failed after %lldms (%u bytes)", (long long)(parse_us / 1000), (unsigned)pos);
      }
      continue;
    }
    if (parse_us > 10000) {
      ESP_LOGW(TAG, "tcp_notify: cJSON_Parse took %lldms (%u bytes)", (long long)(parse_us / 1000), (unsigned)pos);
    }

    cJSON *method_j = cJSON_GetObjectItemCaseSensitive(root, "method");
    cJSON *params   = cJSON_GetObjectItemCaseSensitive(root, "params");
    if (!cJSON_IsString(method_j) || !method_j->valuestring) {
      cJSON_Delete(root);
      continue;
    }
    const char *method = method_j->valuestring;

    if (strcmp(method, "Stream.OnUpdate") == 0) {
      cJSON *stream = cJSON_GetObjectItemCaseSensitive(params, "stream");
      cJSON *status = cJSON_GetObjectItemCaseSensitive(stream, "status");
      if (cJSON_IsString(status) && status->valuestring) {
        if (strcmp(status->valuestring, "playing") == 0) {
          resumed_from_idle = true;
          self->state = media_player::MEDIA_PLAYER_STATE_PLAYING;
          self->pos_last_tick_ms_ = millis();
          self->publish_state();
          ESP_LOGD(TAG, "tcp_notify: OnUpdate playing");
        } else {
          resumed_from_idle = false;
          self->state = media_player::MEDIA_PLAYER_STATE_IDLE;
          self->publish_state();

          if (self->meta_q_hdl_) {
            SnapMetaUpdate clear{};
            clear.is_clear = true;
            xQueueOverwrite(self->meta_q_hdl_, &clear);
          }
          ESP_LOGD(TAG, "tcp_notify: OnUpdate %s", status->valuestring);
        }
      }
    } else if (strcmp(method, "Stream.OnProperties") == 0) {
      cJSON *props    = cJSON_GetObjectItemCaseSensitive(params, "properties");
      cJSON *metadata = cJSON_GetObjectItemCaseSensitive(props, "metadata");

      if (self->meta_q_hdl_ && cJSON_IsObject(metadata)) {
        SnapMetaUpdate meta{};
        meta.duration_s = -1.0f;
        meta.position_s = -1.0f;
        meta.is_clear   = false;
        meta.reset_position = !resumed_from_idle;

        copy_json_string_or_first_array(metadata, "title",  meta.title,  sizeof(meta.title));
        copy_json_string_or_first_array(metadata, "artist", meta.artist, sizeof(meta.artist));
        copy_json_string(metadata, "album",  meta.album,  sizeof(meta.album));
        copy_json_string(metadata, "artUrl", meta.album_art_url, sizeof(meta.album_art_url));
        if (!meta.album_art_url[0])
          copy_json_string(metadata, "album_art_url", meta.album_art_url, sizeof(meta.album_art_url));
        meta.duration_s = copy_json_number(metadata, "duration", -1.0f);

        bool is_dup = (strncmp(meta.title, self->dedup_title_, sizeof(self->dedup_title_)) == 0 &&
                       strncmp(meta.artist, self->dedup_artist_, sizeof(self->dedup_artist_)) == 0);
        if (!is_dup) {
          strncpy(self->dedup_title_,  meta.title,  sizeof(self->dedup_title_)  - 1);
          strncpy(self->dedup_artist_, meta.artist, sizeof(self->dedup_artist_) - 1);
          xQueueOverwrite(self->meta_q_hdl_, &meta);
          ESP_LOGI(TAG, "tcp_notify: title='%s' artist='%s' reset_pos=%d",
                   meta.title, meta.artist, (int)meta.reset_position);
        }
      }
      resumed_from_idle = false;
    }

    cJSON_Delete(root);
  }

  if (sock >= 0) {
    lwip_close(sock);
    self->tcp_notify_sock_ = -1;
  }
  free(line_buf);
  free(rd_buf);
  self->tcp_notify_task_handle_ = nullptr;
  vTaskDelete(nullptr);
}

void SnapClientComponent::net_monitor_task_(void *pvParam) {
  auto *self = static_cast<SnapClientComponent *>(pvParam);

  static bool events_registered = false;
  if (!events_registered) {
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &net_wifi_event_handler_, nullptr);
    esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &net_wifi_event_handler_, nullptr);
    events_registered = true;
  }

  static const char *MON = "net_mon";
  int8_t prev_rssi = 0;
  uint32_t prev_heap = 0;
  uint32_t prev_disc_count = 0;
  int64_t last_alive_us = esp_timer_get_time();
  uint32_t tick_count = 0;

  for (int i = 0; i < 15 && !self->stop_requested_; i++)
    vTaskDelay(pdMS_TO_TICKS(1000));

  while (!self->stop_requested_) {
    vTaskDelay(pdMS_TO_TICKS(5000));
    int64_t now_us = esp_timer_get_time();
    tick_count++;

    int64_t gap_ms = (now_us - last_alive_us) / 1000;
    last_alive_us = now_us;
    if (gap_ms > 8000) {
      ESP_LOGW(MON, "STALL: monitor gap %lldms (expected ~5000)", (long long)gap_ms);
    }

    wifi_ap_record_t ap_info;
    int8_t rssi = 0;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
      rssi = ap_info.rssi;
    }

    if (rssi < -75 && rssi != 0) {
      ESP_LOGW(MON, "RSSI low: %d dBm", (int)rssi);
    } else if (prev_rssi != 0 && abs(rssi - prev_rssi) > 15) {
      ESP_LOGW(MON, "RSSI jump: %d → %d dBm", (int)prev_rssi, (int)rssi);
    }
    prev_rssi = rssi;

    uint32_t free_heap = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    uint32_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    if (free_heap < 20000) {
      ESP_LOGW(MON, "HEAP low: %lu internal, %lu PSRAM",
               (unsigned long)free_heap, (unsigned long)free_psram);
    } else if (prev_heap > 0 && free_heap < prev_heap / 2) {
      ESP_LOGW(MON, "HEAP drop: %lu → %lu internal",
               (unsigned long)prev_heap, (unsigned long)free_heap);
    }
    prev_heap = free_heap;

    uint32_t disc = s_wifi_disc_count;
    if (disc > prev_disc_count) {
      ESP_LOGW(MON, "WiFi disconnects since boot: %lu (new: %lu)",
               (unsigned long)disc, (unsigned long)(disc - prev_disc_count));
      prev_disc_count = disc;
    }

    int pcb_active, pcb_tw;
    decoder_get_tcp_pcb_count(&pcb_active, &pcb_tw);
    if (pcb_tw > 4) {
      ESP_LOGW(MON, "TCP PCBs: active=%d TIME_WAIT=%d (leak?)", pcb_active, pcb_tw);
    }

    int64_t dec_max_loop, dec_max_recv, dec_tsync_us;
    uint32_t dec_tout_cnt, dec_recv_bytes;
    decoder_get_diag(&dec_max_loop, &dec_max_recv, &dec_tsync_us, &dec_tout_cnt, &dec_recv_bytes);
    if (dec_max_recv > 500000) {
      ESP_LOGW(MON, "RECV_SLOW: max_recv=%lldms max_loop=%lldms tout=%lu bytes=%lu",
               (long long)(dec_max_recv / 1000), (long long)(dec_max_loop / 1000),
               (unsigned long)dec_tout_cnt, (unsigned long)dec_recv_bytes);
    }

    if (tick_count % 12 == 0) {
      ESP_LOGD(MON, "ok: rssi=%d heap=%lu psram=%lu pcbs=%d/%d disc=%lu dec_recv=%lldms dec_loop=%lldms tout=%lu",
               (int)rssi, (unsigned long)free_heap, (unsigned long)free_psram,
               pcb_active, pcb_tw, (unsigned long)disc,
               (long long)(dec_max_recv / 1000), (long long)(dec_max_loop / 1000),
               (unsigned long)dec_tout_cnt);
    }

    fr_record(FrEvt::NET_MON,
              (uint16_t)(pcb_tw < 0 ? 0 : (pcb_tw > 65535 ? 65535 : pcb_tw)),
              (int32_t)free_heap, (int32_t)rssi, (int32_t)disc);

  }
  self->net_monitor_task_handle_ = nullptr;
  vTaskDelete(nullptr);
}

}
}

extern "C" void audio_dac_enable(bool  ) {}

#endif
#endif
