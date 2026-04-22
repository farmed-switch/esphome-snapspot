

#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#include "driver/i2s_common.h"
#include "esp_pm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "soc/rtc.h"

#if SOC_I2S_SUPPORTS_APLL
#include "clk_ctrl_os.h"
#endif

#if CONFIG_PM_ENABLE
#include "esp_pm.h"
#endif

#include <math.h>

#include "MedianFilter.h"
#include "TimeFilter.h"
#include "driver/gptimer.h"
#include "driver/i2s_std.h"
#include "player.h"
#include "snapcast.h"

#define USE_SAMPLE_INSERTION CONFIG_USE_SAMPLE_INSERTION

#define SYNC_TASK_PRIORITY 20
#define SYNC_TASK_CORE_ID 1

static const char *TAG = "snapclient";

#if USE_SAMPLE_INSERTION

#if CONFIG_PM_ENABLE
esp_pm_lock_handle_t player_pm_lock_handle = NULL;
#endif

#define INSERT_SAMPLES \
  1

const uint32_t SHORT_OFFSET = 128;
const uint32_t MINI_OFFSET = 64;

#else
const uint32_t SHORT_OFFSET = 2;
const uint32_t MINI_OFFSET = 1;
#endif

static uint32_t apll_normal_predefine[6] = {0, 0, 0, 0, 0, 0};
static uint32_t apll_corr_predefine[][6] = {{0, 0, 0, 0, 0, 0},
                                            {0, 0, 0, 0, 0, 0}};

static SemaphoreHandle_t latencyBufSemaphoreHandle = NULL;
static SemaphoreHandle_t latencyBufFullSemaphoreHandle = NULL;

static gptimer_handle_t gptimer = NULL;

#if USE_TIMEFILTER
static sTimeFilter_t latencyTimeFilter;

static double latencyToServer = 0;
static double latencyDrift = 0;
static int64_t latencyLastUpdate = 0;
#else
static sMedianFilter_t latencyMedianFilter;
static sMedianNode_t latencyMedianLong[LATENCY_MEDIAN_FILTER_LEN];

static int64_t latencyToServer = 0;
#endif

static sMedianFilter_t shortMedianFilter;
static sMedianNode_t shortMedianBuffer[SHORT_BUFFER_LEN];

static sMedianFilter_t miniMedianFilter;
static sMedianNode_t miniMedianBuffer[MINI_BUFFER_LEN];

static int8_t currentDir = 0;

static QueueHandle_t pcmChkQHdl = NULL;

static TaskHandle_t playerTaskHandle = NULL;

static QueueHandle_t snapcastSettingQueueHandle = NULL;

static uint32_t i2sDmaBufCnt;
static uint32_t i2sDmaBufMaxLen;

static SemaphoreHandle_t snapcastSettingsMux = NULL;
static snapcastSetting_t currentSnapcastSetting;

static void tg0_timer_init(void);
static void tg0_timer_deinit(void);

static bool gpTimerRunning = false;

static void player_task(void *pvParameters);

bool gotSettings = false;
bool playerstarted = false;
static volatile bool player_stop_requested = false;

extern void audio_set_mute(bool mute, const char *tag);
extern void audio_dac_enable(bool enabled);

static player_write_cb_t s_write_cb = NULL;
static void *s_write_ctx = NULL;

static i2s_chan_handle_t tx_chan = NULL;
static bool i2sEnabled = false;

static volatile int64_t s_tele_age_us          = 0;
static volatile int64_t s_tele_short_median_us = 0;
static volatile int64_t s_tele_mini_median_us  = 0;
static volatile int      s_tele_queue_depth    = 0;
static volatile int      s_tele_dir            = 0;
static volatile int64_t  s_tele_inserted       = 0;
static volatile int      s_tele_initial_sync   = 0;

static volatile bool s_resync_requested = false;

void player_get_telemetry(player_telemetry_t *out) {
  out->age_us           = s_tele_age_us;
  out->short_median_us  = s_tele_short_median_us;
  out->mini_median_us   = s_tele_mini_median_us;
  out->queue_depth      = s_tele_queue_depth;
  out->dir              = s_tele_dir;
  out->inserted_samples = s_tele_inserted;
  out->initial_sync     = s_tele_initial_sync;
}

double player_get_latency_drift(void) {
#if USE_TIMEFILTER
  return latencyDrift;
#else
  return 0.0;
#endif
}

void player_request_resync(void) {
  s_resync_requested = true;
}

static int64_t MIN(int64_t x, int64_t y) {
    return (x < y) ? x : y;
}

esp_err_t my_i2s_channel_disable(i2s_chan_handle_t handle) {
  if (tx_chan != NULL) {
    if (i2sEnabled == true) {
      i2sEnabled = false;

      return i2s_channel_disable(handle);
    }
  }

  return ESP_OK;
}

esp_err_t my_i2s_channel_enable(i2s_chan_handle_t handle) {
  if (tx_chan != NULL) {
    if (i2sEnabled == false) {
      i2sEnabled = true;

      return i2s_channel_enable(handle);
    }
  }

  return ESP_OK;
}

static void ensure_noiseless(i2s_chan_handle_t tx) {

  static bool i2s_primed = false;
  if (i2s_primed) {
    return;
  }
  i2s_primed = true;

  ESP_LOGI(TAG, "Priming I2S with silence to avoid noise");

  my_i2s_channel_enable(tx);

  size_t silence_size = 44100 * 2 * 2 / 10;
  char *silence_buf = calloc(1, silence_size);

  if (silence_buf) {
    size_t bytes_written;

    i2s_channel_write(tx, silence_buf, silence_size, &bytes_written,
                      pdMS_TO_TICKS(200));

    vTaskDelay(pdMS_TO_TICKS(150));

    free(silence_buf);
    ESP_LOGI(TAG, "Audio path primed with %d bytes of silence", bytes_written);
  }

  my_i2s_channel_disable(tx);
}

static esp_err_t player_setup_i2s(snapcastSetting_t *setting) {

  int32_t sr = setting->sr;
  if (sr == 0) {
    sr = 44100;
  }

  uint32_t chkInFrames = setting->chkInFrames;
  if (chkInFrames == 0) {
    chkInFrames = 1152;
  }

#if USE_SAMPLE_INSERTION
  i2sDmaBufCnt    = 2;
  i2sDmaBufMaxLen = 1023;
#else

  i2sDmaBufCnt    = 2;
  i2sDmaBufMaxLen = 1023;
#endif

  ESP_LOGI(TAG, "player_setup_i2s (no-I2S): sr=%ld chk=%lu dma_len=%ld dma_cnt=%ld",
           sr, chkInFrames, i2sDmaBufMaxLen, i2sDmaBufCnt);

  return 0;
}

static int destroy_pcm_queue(QueueHandle_t *queueHandle) {
  int ret = pdPASS;
  pcm_chunk_message_t *chnk = NULL;

  if (*queueHandle == NULL) {
    ESP_LOGV(TAG, "no pcm chunk queue created?");
    ret = pdFAIL;
  } else {

    while (uxQueueMessagesWaiting(*queueHandle)) {
      ret = xQueueReceive(*queueHandle, &chnk, pdMS_TO_TICKS(2000));
      if (ret != pdFAIL) {
        if (chnk != NULL) {
          free_pcm_chunk(chnk);
        }
      }
      else {
        ESP_LOGE(TAG, "%s: can't get pcm chunk", __func__);
      }
    }

    vQueueDelete(*queueHandle);
    *queueHandle = NULL;

    ret = pdPASS;
  }

  return ret;
}

int deinit_player(void) {
  int ret = 0;

  my_i2s_channel_disable(tx_chan);

  for(int i = 0; i< 100; i++) {
    if (playerstarted) {
      vTaskDelay(pdMS_TO_TICKS(100));
    } else {
      break;
    }
  }

  if (playerTaskHandle != NULL) {
    vTaskDelete(playerTaskHandle);
    playerTaskHandle = NULL;
  }

  if (tx_chan) {
    i2s_del_channel(tx_chan);
    tx_chan = NULL;
  }

  if (snapcastSettingsMux != NULL) {
    vSemaphoreDelete(snapcastSettingsMux);
    snapcastSettingsMux = NULL;
  }
  ret = destroy_pcm_queue(&pcmChkQHdl);

  if (latencyBufSemaphoreHandle != NULL) {
    vSemaphoreDelete(latencyBufSemaphoreHandle);
    latencyBufSemaphoreHandle = NULL;
  }

  if (latencyBufFullSemaphoreHandle != NULL) {
    vSemaphoreDelete(latencyBufFullSemaphoreHandle);
    latencyBufFullSemaphoreHandle = NULL;
  }

  tg0_timer_deinit();

#if CONFIG_PM_ENABLE
  if (player_pm_lock_handle) {
    esp_pm_lock_delete(player_pm_lock_handle);
    player_pm_lock_handle = NULL;
  }
#endif

  ESP_LOGI(TAG, "deinit player done");

  return ret;
}

bool is_player_running(void) {
  return playerstarted;
}

void player_request_stop(void) {
  player_stop_requested = true;

  if (pcmChkQHdl) {
    pcm_chunk_message_t *sentinel = NULL;
    xQueueSend(pcmChkQHdl, &sentinel, 0);
  }
}

int init_player(player_write_cb_t write_cb, void *ctx) {
  int ret = 0;

  deinit_player();

  player_stop_requested = false;
  s_write_cb  = write_cb;
  s_write_ctx = ctx;

  currentSnapcastSetting.buf_ms = 0;
  currentSnapcastSetting.chkInFrames = 0;
  currentSnapcastSetting.codec = NONE;
  currentSnapcastSetting.sr = 0;
  currentSnapcastSetting.ch = 0;
  currentSnapcastSetting.bits = 0;
  currentSnapcastSetting.muted = true;
  currentSnapcastSetting.volume = 0;

  if (snapcastSettingsMux == NULL) {
    snapcastSettingsMux = xSemaphoreCreateMutex();
    xSemaphoreGive(snapcastSettingsMux);
  }

  if (latencyBufSemaphoreHandle == NULL) {
    latencyBufSemaphoreHandle = xSemaphoreCreateMutex();
  }

  if (latencyBufFullSemaphoreHandle == NULL) {
    latencyBufFullSemaphoreHandle = xSemaphoreCreateBinary();
  }
  xSemaphoreTake(latencyBufFullSemaphoreHandle, 0);

#if USE_TIMEFILTER

  TIMEFILTER_Init(&latencyTimeFilter, 0.01, 0.0, 1.001, 0.75, 100, 2.0);
#else

  latencyMedianFilter.numNodes = LATENCY_MEDIAN_FILTER_LEN;
  latencyMedianFilter.medianBuffer = latencyMedianLong;
  reset_latency_buffer();
#endif
  shortMedianFilter.numNodes = SHORT_BUFFER_LEN;
  shortMedianFilter.medianBuffer = shortMedianBuffer;
  MEDIANFILTER_Init(&shortMedianFilter);

  miniMedianFilter.numNodes = MINI_BUFFER_LEN;
  miniMedianFilter.medianBuffer = miniMedianBuffer;
  MEDIANFILTER_Init(&miniMedianFilter);

  #if CONFIG_PM_ENABLE
  esp_pm_lock_create(ESP_PM_APB_FREQ_MAX, 0, "player", &player_pm_lock_handle);
  #endif

  ESP_LOGI(TAG, "init player done");

  return 0;
}

int start_player(snapcastSetting_t *setting) {
    if (playerstarted){
        return -1;
    }
    playerstarted = true;
  int ret = 0;

  ret = player_setup_i2s(setting);
  if (ret < 0) {
    ESP_LOGE(TAG, "player_setup_i2s failed: %d", ret);
    playerstarted = false;
    return -1;
  }

  tg0_timer_init();

#if CONFIG_PM_ENABLE
  ESP_LOGI(TAG, "reset Latency buffer");

  while(reset_latency_buffer()<0) {
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  esp_pm_lock_acquire(player_pm_lock_handle);
#endif

  snapcastSettingQueueHandle = xQueueCreate(1, sizeof(uint8_t));

  if (pcmChkQHdl == NULL)
  {
    snapcastSetting_t scSet;
    memset(&scSet, 0, sizeof(snapcastSetting_t));
    player_get_snapcast_settings(&scSet);

    uint32_t chkInFrames = scSet.chkInFrames;
    if (chkInFrames == 0) {
      chkInFrames = 1152;
    }

    int entries = ceil(((float)scSet.sr / (float)chkInFrames) *
                        ((float)scSet.buf_ms / 1000));

    entries -= ((i2sDmaBufMaxLen * i2sDmaBufCnt) / chkInFrames);

    pcmChkQHdl = xQueueCreate(entries, sizeof(pcm_chunk_message_t *));

    ESP_LOGI(TAG, "created new queue with %d", entries);
  }

  ESP_LOGI(TAG, "Start player_task");

  xTaskCreatePinnedToCore(player_task, "player", 1024 * 5, NULL,
                          SYNC_TASK_PRIORITY, &playerTaskHandle,
                          SYNC_TASK_CORE_ID);

  ESP_LOGI(TAG, "start player done");

  return 0;
}

int8_t player_set_snapcast_settings(snapcastSetting_t *setting) {
  int8_t ret = pdPASS;

  xSemaphoreTake(snapcastSettingsMux, portMAX_DELAY);

  memcpy(&currentSnapcastSetting, setting, sizeof(snapcastSetting_t));

  xSemaphoreGive(snapcastSettingsMux);

  return ret;
}

int8_t player_get_snapcast_settings(snapcastSetting_t *setting) {
  int8_t ret = pdPASS;

  xSemaphoreTake(snapcastSettingsMux, portMAX_DELAY);

  memcpy(setting, &currentSnapcastSetting, sizeof(snapcastSetting_t));

  xSemaphoreGive(snapcastSettingsMux);

  return ret;
}

#if USE_TIMEFILTER

int32_t player_latency_insert(int64_t newValue, int64_t max_error, int64_t time_added) {
  TIMEFILTER_Insert(&latencyTimeFilter, newValue, max_error, time_added);
  int64_t last_update_ = latencyTimeFilter.last_update_;
  double offset_ = latencyTimeFilter.offset_;
  double drift_ = latencyTimeFilter.use_drift_ ? latencyTimeFilter.drift_ : 0.0;
  if (xSemaphoreTake(latencyBufSemaphoreHandle, pdMS_TO_TICKS(0)) == pdTRUE) {
    if (TIMEFILTER_isFull(&latencyTimeFilter, LATENCY_TIME_FILTER_FULL)) {
      xSemaphoreGive(latencyBufFullSemaphoreHandle);

    }

    latencyToServer = offset_;
    latencyDrift = drift_;
    latencyLastUpdate = last_update_;

    xSemaphoreGive(latencyBufSemaphoreHandle);
  } else {
    ESP_LOGW(TAG, "couldn't set latencyToServer = medianValue");
  }

  return 0;
}
#else

int32_t player_latency_insert(int64_t newValue) {
  int64_t medianValue;

  medianValue = MEDIANFILTER_Insert(&latencyMedianFilter, newValue);
  if (xSemaphoreTake(latencyBufSemaphoreHandle, pdMS_TO_TICKS(0)) == pdTRUE) {
    if (MEDIANFILTER_isFull(&latencyMedianFilter, LATENCY_MEDIAN_FILTER_FULL)) {
      xSemaphoreGive(latencyBufFullSemaphoreHandle);

    }

    latencyToServer = medianValue;

    xSemaphoreGive(latencyBufSemaphoreHandle);
  } else {
    ESP_LOGW(TAG, "couldn't set latencyToServer = medianValue");
  }

  return 0;
}
#endif

int32_t player_send_snapcast_setting(snapcastSetting_t *setting) {
  int ret;
  snapcastSetting_t curSet;
  uint8_t settingChanged = 1;

  ret = player_get_snapcast_settings(&curSet);

  if ((curSet.bits != setting->bits) || (curSet.buf_ms != setting->buf_ms) ||
      (curSet.ch != setting->ch) ||
      (curSet.chkInFrames != setting->chkInFrames) ||
      (curSet.codec != setting->codec) || (curSet.muted != setting->muted) ||
      (curSet.sr != setting->sr) || (curSet.volume != setting->volume) ||
      (curSet.cDacLat_ms != setting->cDacLat_ms)) {
    ret = player_set_snapcast_settings(setting);
    if (ret != pdPASS) {
      ESP_LOGE(TAG,
               "player_send_snapcast_setting: couldn't change "
               "snapcast setting");
    }

    if ((playerTaskHandle != NULL) && (snapcastSettingQueueHandle != NULL)) {
      ret = xQueueOverwrite(snapcastSettingQueueHandle, &settingChanged);
      if (ret != pdPASS) {
        ESP_LOGE(TAG,
                  "player_send_snapcast_setting: couldn't notify "
                  "snapcast setting");
      } else {
                  ESP_LOGI(TAG,
                  "got settings and notified player_task");
      }
    }
  }

  if (!gotSettings && (setting->bits > 0) && ( setting->buf_ms > 0) && (setting->ch > 0) &&
      (setting->chkInFrames > 0) && (setting->sr > 0)) {
    gotSettings = true;
  }

  return pdPASS;
}

#if USE_TIMEFILTER

int32_t reset_latency_buffer(void) {

  TIMEFILTER_Reset(&latencyTimeFilter);

  if (latencyBufSemaphoreHandle == NULL) {
    ESP_LOGE(TAG, "reset_diff_buffer: latencyBufSemaphoreHandle == NULL");

    return -2;
  }
  xSemaphoreTake(latencyBufFullSemaphoreHandle, pdMS_TO_TICKS(10));
  if (xSemaphoreTake(latencyBufSemaphoreHandle, pdMS_TO_TICKS(100)) == pdTRUE) {
    latencyToServer = 0;
    latencyDrift = 0;
    latencyLastUpdate = 0;

    xSemaphoreGive(latencyBufSemaphoreHandle);
  } else {
    ESP_LOGW(TAG, "reset_diff_buffer: can't take semaphore");

    return -1;
  }

  return 0;
}

int32_t latency_buffer_full(bool *is_full) {
  *is_full = TIMEFILTER_isFull(&latencyTimeFilter, LATENCY_TIME_FILTER_FULL);
  return 0;
}

int32_t get_diff_to_server(int64_t *tDiff, int64_t now) {
  static double lastDiff = 0;
  static double lastDrift = 0;
  static int64_t lastLastUpdate = 0;

  if (latencyBufSemaphoreHandle == NULL) {
    ESP_LOGE(TAG, "get_diff_to_server: latencyBufSemaphoreHandle == NULL");

    return -2;
  }

  double dt;
  int64_t offset;
  if (xSemaphoreTake(latencyBufSemaphoreHandle, 0) == pdFALSE) {
    dt = now - lastLastUpdate;
    offset = round(lastDiff + lastDrift * dt);
    *tDiff = offset;

    return -1;
  }

  dt = now - latencyLastUpdate;
  offset = round(latencyToServer + latencyDrift * dt);

  *tDiff = offset;
  lastLastUpdate = latencyLastUpdate;
  lastDrift = latencyDrift;
  lastDiff = latencyToServer;

  xSemaphoreGive(latencyBufSemaphoreHandle);

  return 0;
}

#else

int32_t reset_latency_buffer(void) {
  xSemaphoreTake(latencyBufFullSemaphoreHandle, pdMS_TO_TICKS(10));

  if (MEDIANFILTER_Init(&latencyMedianFilter) < 0) {
    ESP_LOGE(TAG, "reset_diff_buffer: couldn't init median filter long. STOP");

    return -2;
  }

  if (latencyBufSemaphoreHandle == NULL) {
    ESP_LOGE(TAG, "reset_diff_buffer: latencyBufSemaphoreHandle == NULL");

    return -2;
  }

  if (xSemaphoreTake(latencyBufSemaphoreHandle, portMAX_DELAY) == pdTRUE) {
    latencyToServer = 0;

    xSemaphoreGive(latencyBufSemaphoreHandle);
  } else {
    ESP_LOGW(TAG, "reset_diff_buffer: can't take semaphore");

    return -1;
  }

  return 0;
}

int32_t latency_buffer_full(bool *is_full) {
  *is_full = MEDIANFILTER_isFull(&latencyMedianFilter, LATENCY_MEDIAN_FILTER_FULL);

  return 0;
}

int32_t get_diff_to_server(int64_t *tDiff, int64_t now) {
  static int64_t lastDiff = 0;

  if (latencyBufSemaphoreHandle == NULL) {
    ESP_LOGE(TAG, "get_diff_to_server: latencyBufSemaphoreHandle == NULL");

    return -2;
  }

  if (xSemaphoreTake(latencyBufSemaphoreHandle, 0) == pdFALSE) {
    *tDiff = lastDiff;

    return -1;
  }

  *tDiff = latencyToServer;
  lastDiff = latencyToServer;

  xSemaphoreGive(latencyBufSemaphoreHandle);

  return 0;
}
#endif

int32_t server_now(int64_t *sNow, int64_t *diff2Server) {
  int64_t diff, now;

  if (sNow == NULL) {
    return -2;
  }

  now = esp_timer_get_time();

  if (get_diff_to_server(&diff, now) == -1) {

  }

  if (diff == 0) {

    return -1;
  }

  *sNow = now + diff;

  if (diff2Server) {
    *diff2Server = diff;
  }

  return 0;
}

static bool IRAM_ATTR timer_group0_alarm_cb(
    gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata,
    void *user_data) {

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  uint64_t timer_counter_value = edata->count_value;

  xTaskNotifyFromISR(playerTaskHandle, (uint32_t)timer_counter_value,
                     eSetValueWithOverwrite, &xHigherPriorityTaskWoken);

  return xHigherPriorityTaskWoken == pdTRUE;
}

esp_err_t my_gptimer_stop(gptimer_handle_t timer) {
  if (gpTimerRunning == true) {
    gpTimerRunning = false;

    esp_err_t ret = 0;
    ret |= gptimer_stop(timer);
    ret |= gptimer_disable(timer);

    return ret;
  }

  return ESP_OK;
}

esp_err_t my_gptimer_start(gptimer_handle_t timer) {
  if (gpTimerRunning == false) {
    gpTimerRunning = true;

    return gptimer_start(timer);
  }

  return ESP_OK;
}

static void tg0_timer_deinit(void) {

  if (gptimer) {
    ESP_ERROR_CHECK(my_gptimer_stop(gptimer));
    ESP_ERROR_CHECK(gptimer_del_timer(gptimer));
    gptimer = NULL;
  }
}

static void tg0_timer_init(void) {
  tg0_timer_deinit();

  gptimer_config_t timer_config = {
      .clk_src = GPTIMER_CLK_SRC_DEFAULT,
      .direction = GPTIMER_COUNT_UP,
      .resolution_hz = 1000000,
  };
  ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

  gptimer_event_callbacks_t cbs = {
      .on_alarm = timer_group0_alarm_cb,
  };
  ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

  ESP_LOGI(TAG, "init initial sync timer");
}

static void tg0_timer1_start(uint64_t alarm_value) {
  if (gptimer) {
    my_gptimer_stop(gptimer);
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    ESP_ERROR_CHECK(gptimer_set_raw_count(gptimer, 0));
    gptimer_alarm_config_t alarm_config1 = {
        .alarm_count = alarm_value,
        .reload_count = 0,
        .flags.auto_reload_on_alarm = false,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config1));
    ESP_ERROR_CHECK(my_gptimer_start(gptimer));
  }

}

#if !USE_SAMPLE_INSERTION

void adjust_apll(int8_t direction) {
  int sdm0, sdm1, sdm2, o_div;

  if (currentDir == direction) {
    return;
  }

  if (direction == 1) {

    sdm0 = apll_corr_predefine[0][2];
    sdm1 = apll_corr_predefine[0][3];
    sdm2 = apll_corr_predefine[0][4];
    o_div = apll_corr_predefine[0][5];
  } else if (direction == -1) {

    sdm0 = apll_corr_predefine[1][2];
    sdm1 = apll_corr_predefine[1][3];
    sdm2 = apll_corr_predefine[1][4];
    o_div = apll_corr_predefine[1][5];
  } else {

    sdm0 = apll_normal_predefine[2];
    sdm1 = apll_normal_predefine[3];
    sdm2 = apll_normal_predefine[4];
    o_div = apll_normal_predefine[5];

    direction = 0;
  }

  rtc_clk_apll_coeff_set(o_div, sdm0, sdm1, sdm2);

  currentDir = direction;
}
#endif

int8_t free_pcm_chunk_fragments(pcm_chunk_fragment_t *fragment) {
  if (fragment == NULL) {
    ESP_LOGE(TAG, "free_pcm_chunk_fragments() parameter Error");

    return -1;
  }

  if (fragment->nextFragment == NULL) {
    if (fragment->payload != NULL) {
      free(fragment->payload);
      fragment->payload = NULL;
    }

    free(fragment);
    fragment = NULL;
  } else {
    free_pcm_chunk_fragments(fragment->nextFragment);
  }

  return 0;
}

int8_t free_pcm_chunk(pcm_chunk_message_t *pcmChunk) {
  if (pcmChunk == NULL) {
    ESP_LOGE(TAG, "free_pcm_chunk() parameter Error");

    return -1;
  }

  free_pcm_chunk_fragments(pcmChunk->fragment);
  pcmChunk->fragment = NULL;

  free(pcmChunk);
  pcmChunk = NULL;

  return 0;
}

int32_t allocate_pcm_chunk_memory_caps(pcm_chunk_message_t *pcmChunk,
                                       size_t bytes, uint32_t caps) {
  size_t largestFreeBlock, freeMem;
  int ret = -3;

  pcmChunk->caps = caps;

  if (caps != 0) {
    freeMem = heap_caps_get_free_size(caps);
    largestFreeBlock = heap_caps_get_largest_free_block(caps);
    if ((freeMem >= bytes) && (largestFreeBlock >= bytes)) {

      pcmChunk->fragment->payload = (char *)heap_caps_malloc(bytes, caps);
      if (pcmChunk->fragment->payload == NULL) {
        ESP_LOGD(TAG, "Failed to allocate %d bytes of %s for pcm chunk payload",
                 bytes,
                 (caps == (MALLOC_CAP_32BIT | MALLOC_CAP_EXEC)) ? ("IRAM")
                                                                : ("DRAM"));

        ret = -2;
      } else {
        pcmChunk->totalSize = bytes;
        pcmChunk->fragment->nextFragment = NULL;
        pcmChunk->fragment->size = bytes;

        ret = 0;
      }
    } else {

    }
  } else {
    pcmChunk->fragment->payload = (char *)malloc(bytes);
    if (pcmChunk->fragment->payload == NULL) {
      ESP_LOGE(TAG, "Failed to malloc memory for pcm chunk payload");

      ret = -2;
    } else {
      pcmChunk->totalSize = bytes;
      pcmChunk->fragment->nextFragment = NULL;
      pcmChunk->fragment->size = bytes;

      ret = 0;
    }
  }

  return ret;
}

int32_t allocate_pcm_chunk_memory_caps_fragmented(pcm_chunk_message_t *pcmChunk,
                                                  size_t bytes, uint32_t caps) {
  size_t largestFreeBlock, freeMem;
  int ret = -3;

  freeMem = heap_caps_get_free_size(caps);
  largestFreeBlock = heap_caps_get_largest_free_block(caps);
  if (freeMem >= bytes) {

    if (largestFreeBlock >= bytes) {
      pcmChunk->fragment->payload = (char *)heap_caps_malloc(bytes, caps);
      if (pcmChunk->fragment->payload == NULL) {
        ESP_LOGE(TAG, "Failed to allocate IRAM memory for pcm chunk payload");

        ret = -2;
      } else {
        pcmChunk->totalSize = bytes;
        pcmChunk->fragment->nextFragment = NULL;
        pcmChunk->fragment->size = bytes;

        ret = 0;
      }
    } else {
      size_t remainingBytes = bytes + (largestFreeBlock % 4);
      size_t needBytes = largestFreeBlock - (largestFreeBlock % 4);
      pcm_chunk_fragment_t *fragment = pcmChunk->fragment;

      pcmChunk->totalSize = 0;

      while (remainingBytes) {
        fragment->payload = (char *)heap_caps_malloc(needBytes, caps);
        if (fragment->payload == NULL) {
          ESP_LOGE(TAG,
                   "Failed to allocate fragmented IRAM memory for "
                   "pcm chunk payload %d %d %d %d",
                   needBytes, remainingBytes, heap_caps_get_free_size(caps),
                   heap_caps_get_largest_free_block(caps));

          ret = -2;

          break;
        } else {
          fragment->size = needBytes;
          remainingBytes -= needBytes;
          pcmChunk->totalSize += needBytes;

          if (remainingBytes > 0) {
            fragment->nextFragment =
                (pcm_chunk_fragment_t *)calloc(1, sizeof(pcm_chunk_fragment_t));
            if (fragment->nextFragment == NULL) {
              ESP_LOGE(TAG,
                       "Failed to fragmented IRAM memory "
                       "for pcm chunk fragment");

              ret = -2;

              break;
            } else {
              fragment = fragment->nextFragment;
              largestFreeBlock = heap_caps_get_largest_free_block(caps);
              if (largestFreeBlock >= remainingBytes) {
                needBytes = remainingBytes;
              } else {
                needBytes = largestFreeBlock - (largestFreeBlock % 4);
              }
            }
          } else {
            ret = 0;
          }
        }
      }
    }
  } else {

  }

  return ret;
}

int32_t allocate_pcm_chunk_memory(pcm_chunk_message_t **pcmChunk,
                                  size_t bytes) {
  int ret = -3;

  static uint32_t s_alloc_calls_total  = 0;
  static uint32_t s_alloc_slow_calls   = 0;
  static uint32_t s_alloc_retries_sum  = 0;
  static uint32_t s_alloc_max_retries  = 0;
  static int64_t  s_alloc_stats_t0_us  = 0;

  s_alloc_calls_total++;
  if (s_alloc_stats_t0_us == 0) {
    s_alloc_stats_t0_us = esp_timer_get_time();
  }

  *pcmChunk = (pcm_chunk_message_t *)calloc(1, sizeof(pcm_chunk_message_t));
  if (*pcmChunk == NULL) {
    ESP_LOGE(TAG, "Failed to allocate memory for pcm chunk message");

    return -2;
  }

  (*pcmChunk)->fragment =
      (pcm_chunk_fragment_t *)calloc(1, sizeof(pcm_chunk_fragment_t));
  if ((*pcmChunk)->fragment == NULL) {
    ESP_LOGE(TAG, "Failed to allocate memory for pcm chunk fragment");

    free_pcm_chunk(*pcmChunk);

    return -2;
  }

#if CONFIG_SPIRAM && CONFIG_SPIRAM_BOOT_INIT
  ret = allocate_pcm_chunk_memory_caps(*pcmChunk, bytes,
                                       MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
#elif CONFIG_SPIRAM
  ret = allocate_pcm_chunk_memory_caps(*pcmChunk, bytes, 0);
#else

  uint32_t x = 50;
  uint32_t retries_this_call = 0;
  for (int i = 0; i < x; i++) {
    ret = allocate_pcm_chunk_memory_caps(*pcmChunk, bytes,
                                         MALLOC_CAP_32BIT | MALLOC_CAP_EXEC);
    if (ret < 0) {
      ret = allocate_pcm_chunk_memory_caps(*pcmChunk, bytes, MALLOC_CAP_8BIT);

    }

    if (ret < 0) {
      retries_this_call++;
      vTaskDelay(pdMS_TO_TICKS(1));
    } else {
      break;
    }
  }

  s_alloc_retries_sum += retries_this_call;
  if (retries_this_call > s_alloc_max_retries) {
    s_alloc_max_retries = retries_this_call;
  }
  if (retries_this_call >= 5) {
    s_alloc_slow_calls++;
    ESP_LOGW(TAG,
             "PCM_ALLOC_SLOW: retries=%lu bytes=%u "
             "free8=%u largest8=%u free32=%u largest32=%u",
             (unsigned long)retries_this_call, (unsigned)bytes,
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_32BIT | MALLOC_CAP_EXEC),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_32BIT | MALLOC_CAP_EXEC));
  }

  {
    int64_t now_us = esp_timer_get_time();
    if ((now_us - s_alloc_stats_t0_us) >= 60000000LL) {
      ESP_LOGI(TAG,
               "PCM_ALLOC_STATS (60s): calls=%lu slow>=5=%lu "
               "retries_sum=%lu max_retries=%lu "
               "free8=%u largest8=%u",
               (unsigned long)s_alloc_calls_total,
               (unsigned long)s_alloc_slow_calls,
               (unsigned long)s_alloc_retries_sum,
               (unsigned long)s_alloc_max_retries,
               (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
               (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
      s_alloc_calls_total = 0;
      s_alloc_slow_calls  = 0;
      s_alloc_retries_sum = 0;
      s_alloc_max_retries = 0;
      s_alloc_stats_t0_us = now_us;
    }
  }
#endif

  if (ret < 0) {
    ESP_LOGW(TAG,
             "couldn't get memory to insert chunk, inserting an chunk "
             "containing just 0");

    ESP_LOGW(
        TAG, "%d, %d, %d, %d", heap_caps_get_free_size(MALLOC_CAP_8BIT),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
        heap_caps_get_free_size(MALLOC_CAP_32BIT | MALLOC_CAP_EXEC),
        heap_caps_get_largest_free_block(MALLOC_CAP_32BIT | MALLOC_CAP_EXEC));

    (*pcmChunk)->fragment->payload = NULL;
    (*pcmChunk)->totalSize = bytes;
    (*pcmChunk)->fragment->nextFragment = NULL;
    (*pcmChunk)->fragment->size = bytes;

    ret = 0;
  } else {

  }

  return ret;
}

int32_t insert_pcm_chunk(pcm_chunk_message_t *pcmChunk) {
  if (pcmChunk == NULL) {
    ESP_LOGE(TAG, "Parameter Error");

    return -1;
  }

  if (pcmChkQHdl == NULL) {
    ESP_LOGW(TAG, "pcm chunk queue not created. Player started: %s", playerstarted ? "True": "False");

    free_pcm_chunk(pcmChunk);

    snapcastSetting_t curSet;
    player_get_snapcast_settings(&curSet);
    if (!curSet.muted && gotSettings) {
        start_player(&curSet);
    }

    return -2;
  }

  bool isFull = false;
  latency_buffer_full(&isFull);
  if (isFull == false) {
    free_pcm_chunk(pcmChunk);

    ESP_LOGD(TAG, "INSERT_DROP: latency buffer not ready, chunk discarded");

    return -3;
  }

  if (xQueueSend(pcmChkQHdl, &pcmChunk, pdMS_TO_TICKS(1)) != pdTRUE) {
    ESP_LOGD(TAG, "INSERT_DROP: queue full, q=%d, chunk discarded",
             (int)uxQueueMessagesWaiting(pcmChkQHdl));

    free_pcm_chunk(pcmChunk);
  }

  return 0;
}

int32_t pcm_chunk_queue_msg_waiting(void) {
  int ret = 0;

  if (pcmChkQHdl) {
    ret = uxQueueMessagesWaiting(pcmChkQHdl);
  }

  return ret;
}

static bool audioCodecCanSleep = false;

static void player_task(void *pvParameters) {
  pcm_chunk_message_t *chnk = NULL;
  int64_t age;
  int64_t serverNow = 0;
  BaseType_t ret;
  int64_t chunkDuration_us = 24000;
  char *p_payload = NULL;
  size_t size = 0;
  uint32_t notifiedValue;
  snapcastSetting_t scSet;
  uint8_t scSetChgd = 0;
  uint64_t timer_val;
  int initialSync = 0;
  int empty_q_count = 0;
  int dir = 0;
  int32_t dir_insert_sample = 0;
  int64_t insertedSamplesCounter = 0;
  int64_t buf_us = 0;
  pcm_chunk_fragment_t *fragment = NULL;
  size_t written;
  int64_t clientDacLatency_us = 0;
  int64_t diff2Server = 0;
  int64_t outputBufferDacTime_us = 0;
  int64_t dmaDescDuration_us = 0;
  size_t alreadyWritten = 0;
  static uint32_t queueCreatedWithChkInFrames = UINT32_MAX;
  int64_t playback_start_time_us = 0;
  uint64_t samples_written = 0;
  int64_t prev_chunk_end_us = 0;
  int64_t abs_check_time = 0;
  UBaseType_t uxHighWaterMark;

  memset(&scSet, 0, sizeof(snapcastSetting_t));
  player_get_snapcast_settings(&scSet);

  ESP_LOGI(TAG, "started sync task");

  queueCreatedWithChkInFrames = scSet.chkInFrames;

  initialSync = 0;

  ESP_LOGW(TAG, "MUTE: INIT (player start)");
  audio_set_mute(true, "init");

  buf_us = (int64_t)(scSet.buf_ms) * 1000LL;
  clientDacLatency_us = (int64_t)scSet.cDacLat_ms * 1000LL;
  dmaDescDuration_us =
              1000000LL * (int64_t)i2sDmaBufMaxLen / (int64_t)scSet.sr;
#if !USE_SAMPLE_INSERTION

  currentDir = 1;
  adjust_apll(0);
#endif

  ESP_LOGW(TAG, "MUTE: SERVER_CFG muted=%d", scSet.muted);
  audio_set_mute(scSet.muted, "srv_cfg");

  while (!player_stop_requested) {
    if (xSemaphoreTake(latencyBufFullSemaphoreHandle, pdMS_TO_TICKS(100)) == pdTRUE) {
      xSemaphoreGive(latencyBufFullSemaphoreHandle);
      break;
    }
  }
  while (!player_stop_requested) {
    int64_t tDiff;
    if (get_diff_to_server(&tDiff, esp_timer_get_time())==0) {
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  while (1) {
    if (player_stop_requested) {
      ESP_LOGI(TAG, "player_task: stop requested, flushing queue and exiting");
      break;
    }

    ret = xQueueReceive(snapcastSettingQueueHandle, &scSetChgd, 0);
    if (ret == pdTRUE) {
      snapcastSetting_t __scSet;

      player_get_snapcast_settings(&__scSet);

      if ((__scSet.buf_ms > 0) && (__scSet.chkInFrames > 0) &&
          (__scSet.sr > 0)) {
        buf_us = (int64_t)(__scSet.buf_ms) * 1000LL;

        clientDacLatency_us = (int64_t)__scSet.cDacLat_ms * 1000LL;

        if ((scSet.sr != __scSet.sr) || (scSet.bits != __scSet.bits) ||
            (scSet.ch != __scSet.ch)) {
          my_i2s_channel_enable(tx_chan);
          ESP_LOGW(TAG, "MUTE: FORMAT_CHANGE sr=%ld ch=%d bits=%d", __scSet.sr, __scSet.ch, __scSet.bits);
          audio_set_mute(true, "fmt_chg");
          my_i2s_channel_disable(tx_chan);

          ret = player_setup_i2s(&__scSet);
          if (ret < 0) {
            ESP_LOGE(TAG, "player_setup_i2s failed: %d", ret);

            return;
          }

          dmaDescDuration_us =
              1000000LL * (int64_t)i2sDmaBufMaxLen / (int64_t)__scSet.sr;

#if !USE_SAMPLE_INSERTION

          currentDir = 1;
          adjust_apll(0);
#endif

          initialSync = 0;
          prev_chunk_end_us = 0;
        }
        if ((scSet.buf_ms != __scSet.buf_ms) ||
            (queueCreatedWithChkInFrames > __scSet.chkInFrames)) {
          destroy_pcm_queue(&pcmChkQHdl);
        }

        if (pcmChkQHdl == NULL) {
          int entries = ceil(((float)__scSet.sr / (float)__scSet.chkInFrames) *
                             ((float)__scSet.buf_ms / 1000));

          entries -= (i2sDmaBufMaxLen * i2sDmaBufCnt) / __scSet.chkInFrames;

          queueCreatedWithChkInFrames = __scSet.chkInFrames;

          pcmChkQHdl = xQueueCreate(entries, sizeof(pcm_chunk_message_t *));

          ESP_LOGI(TAG, "created new queue with %d", entries);
        }

        if ((scSet.sr != __scSet.sr) || (scSet.bits != __scSet.bits) ||
            (scSet.ch != __scSet.ch) || (scSet.buf_ms != __scSet.buf_ms)) {
          ESP_LOGI(TAG,
                   "snapserver config changed, buffer %ldms, chunk %ld frames, "
                   "sample rate %ld, ch %d, bits %d mute %d latency %ld",
                   __scSet.buf_ms, __scSet.chkInFrames, __scSet.sr, __scSet.ch,
                   __scSet.bits, __scSet.muted, __scSet.cDacLat_ms);
        } else {
          ESP_LOGW(TAG, "MUTE: SETTING_UPDATE muted=%d", __scSet.muted);
          audio_set_mute(__scSet.muted, "setting");
          ESP_LOGI(TAG, "snapserver config changed, mute: %d", __scSet.muted);
        }

        scSet = __scSet;

      }

    }

    if (chnk == NULL) {
      if (pcmChkQHdl != NULL) {
        ret = xQueueReceive(pcmChkQHdl, &chnk, pdMS_TO_TICKS(player_stop_requested ? 10 : 2000));
      } else {

        vTaskDelay(pdMS_TO_TICKS(100));

        continue;
      }

      if (ret != pdFAIL) {
        if (chnk == NULL) {

          continue;
        }
        chunkDuration_us =
            1000000LL *
            (int64_t)(chnk->totalSize / ((scSet.bits >> 3) * scSet.ch)) /
            (int64_t)scSet.sr;

      }
    } else {

      ret = pdPASS;
    }

    if (ret != pdFAIL) {
      int64_t chunkStart = (int64_t)chnk->timestamp.sec * 1000000LL +
                           (int64_t)chnk->timestamp.usec;

      if (initialSync == 0) {
        s_resync_requested = false;
        if (server_now(&serverNow, &diff2Server) >= 0) {
          age = serverNow - chunkStart - buf_us + clientDacLatency_us;
          ESP_LOGI(TAG, "SYNC0: age=%lldus diff2Server=%lldus buf_us=%lld qWait=%d",
                   age, diff2Server, buf_us,
                   pcmChkQHdl ? (int)uxQueueMessagesWaiting(pcmChkQHdl) : -1);
          s_tele_age_us        = age;
          s_tele_initial_sync  = 0;
          s_tele_queue_depth   = pcmChkQHdl ? (int)uxQueueMessagesWaiting(pcmChkQHdl) : -1;

          if (age >= 10000) {
            int pre_drain_max = 200;
            int pre_drain_count = 0;
            while (pre_drain_max-- > 0) {
              free_pcm_chunk(chnk);
              chnk = NULL;
              pre_drain_count++;
              ret = xQueueReceive(pcmChkQHdl, &chnk, pdMS_TO_TICKS(30));
              if (ret == pdFAIL) { break; }
              int64_t _ncs = (int64_t)chnk->timestamp.sec * 1000000LL +
                             (int64_t)chnk->timestamp.usec;
              if (server_now(&serverNow, &diff2Server) < 0) { break; }
              int64_t _na = serverNow - _ncs - buf_us + clientDacLatency_us;
              if (_na < 0) {
                chunkStart = _ncs;
                age = _na;
                break;
              }
            }
            if (pre_drain_count > 0) {
              ESP_LOGW(TAG, "SYNC0_DRAIN: discarded %d stale chunks, age now=%lldus", pre_drain_count, age);
            }
          }
        } else {
          ESP_LOGW(TAG, "SYNC0_DROP: server_now() failed, chunk discarded");

          if (chnk != NULL) {
            free_pcm_chunk(chnk);
            chnk = NULL;
          }

          vTaskDelay(pdMS_TO_TICKS(1));

          continue;
        }

        if (age < 0) {
          MEDIANFILTER_Init(&shortMedianFilter);
          MEDIANFILTER_Init(&miniMedianFilter);

          tg0_timer1_start(-age);

#if !USE_SAMPLE_INSERTION
          adjust_apll(0);
#endif

          xTaskNotifyWait(pdFALSE,
                          pdFALSE,
                          &notifiedValue,
                          portMAX_DELAY);

          my_gptimer_stop(gptimer);

          audio_dac_enable(true);

          playback_start_time_us = esp_timer_get_time();
          samples_written = i2sDmaBufCnt * i2sDmaBufMaxLen;
          alreadyWritten = 0;

          timer_val = (int64_t)notifiedValue;

          age = (int64_t)timer_val - (-age);

          initialSync = 1;

          vTaskDelay(pdMS_TO_TICKS(2));
          ESP_LOGW(TAG, "MUTE: SYNC_DONE age=%lldus muted=%d iSync=%d", age, scSet.muted, initialSync);
          audio_set_mute(scSet.muted, "sync_done");

          ESP_LOGI(TAG, "initial sync age: %lldus, chunkDur: %lldus, qWait=%d",
                   age, chunkDuration_us,
                   pcmChkQHdl ? (int)uxQueueMessagesWaiting(pcmChkQHdl) : -1);

          {
            int64_t _sn, _d2s;
            if (server_now(&_sn, &_d2s) >= 0) {
              int64_t abs_age = _sn - chunkStart - buf_us + clientDacLatency_us;
              ESP_LOGW(TAG, "ABS_CHECK_SYNC: abs=%lldus rel=%lldus delta=%lldus q=%d",
                       abs_age, age, abs_age - age,
                       pcmChkQHdl ? (int)uxQueueMessagesWaiting(pcmChkQHdl) : -1);
            }
            abs_check_time = esp_timer_get_time() + 10000000;
          }

          if (size == 0) {
            continue;
          }
        } else if (age >= 0) {
          if (chnk != NULL) {
            free_pcm_chunk(chnk);
            chnk = NULL;
          }

          wifi_ap_record_t ap;
          esp_wifi_sta_get_ap_info(&ap);

          my_gptimer_stop(gptimer);

          int msgWaiting = uxQueueMessagesWaiting(pcmChkQHdl);

          ESP_LOGW(TAG,
                   "RESYNCING HARD 1: age %lldus, latency %lldus, free %d, "
                   "largest block %d, rssi: %d, left in queue %d",
                   age, diff2Server, heap_caps_get_free_size(MALLOC_CAP_32BIT),
                   heap_caps_get_largest_free_block(MALLOC_CAP_32BIT), ap.rssi, msgWaiting);

          uint32_t c = ceil((float)age / (float)chunkDuration_us);

          while (c--) {
            ret = xQueueReceive(pcmChkQHdl, &chnk, pdMS_TO_TICKS(1));
            if (ret == pdPASS) {
              free_pcm_chunk(chnk);
              chnk = NULL;
            } else {
              break;
            }
          }

          dir = 0;

          insertedSamplesCounter = 0;

          ESP_LOGW(TAG, "MUTE: HARD1 age=%lldus", age);
          audio_set_mute(true, "hard1");
          my_i2s_channel_disable(tx_chan);

          continue;
        }
      }

      const bool enableControlLoop = true;

      const int64_t shortOffset = SHORT_OFFSET;
      const int64_t miniOffset = MINI_OFFSET;

      const int64_t hardResyncThreshold = 5000;

      if (initialSync == 1) {
        if (size == 0) {
          fragment = chnk->fragment;
          p_payload = fragment->payload;
          size = fragment->size;
        }

        if (prev_chunk_end_us != 0 && size > 0) {
          int64_t gap_us = chunkStart - prev_chunk_end_us;
          if (gap_us > chunkDuration_us + (chunkDuration_us >> 1)) {

            int64_t gap_age = 0;
            int64_t _sNow, _d2s;
            if (server_now(&_sNow, &_d2s) >= 0) {
              int64_t _samples_played = samples_written - i2sDmaBufCnt * i2sDmaBufMaxLen;
              int64_t _target = chunkStart - _d2s + buf_us - outputBufferDacTime_us - clientDacLatency_us;
              int64_t _actual = playback_start_time_us + (_samples_played * 1000000LL / (int64_t)scSet.sr);
              gap_age = _actual - _target;
            }

            int64_t fill_us = -gap_age;
            if (fill_us < 0) fill_us = 0;

            if (fill_us > 500000) fill_us = 500000;

            size_t framesToBytes_gap = (scSet.ch * (scSet.bits >> 3));
            int64_t fill_samples = (fill_us * (int64_t)scSet.sr) / 1000000LL;
            size_t fill_bytes = (size_t)(fill_samples * framesToBytes_gap);

            if (fill_bytes > 0) {
              static uint8_t zero_buf[512];
              memset(zero_buf, 0, sizeof(zero_buf));
              size_t remaining = fill_bytes;

              ESP_LOGW(TAG, "GAP_FILL: gap=%lldus age=%lldus filling %u bytes (%lld samples) of silence",
                       gap_us, gap_age, (unsigned)fill_bytes, fill_samples);

              while (remaining > 0) {
                size_t chunk_sz = remaining < sizeof(zero_buf) ? remaining : sizeof(zero_buf);
                written = 0;
                if (s_write_cb) s_write_cb(zero_buf, chunk_sz, &written, s_write_ctx);
                if (written == 0) {
                  vTaskDelay(pdMS_TO_TICKS(1));
                  continue;
                }
                samples_written += (written / framesToBytes_gap);
                remaining -= written;
              }
            }

            MEDIANFILTER_Init(&shortMedianFilter);
            MEDIANFILTER_Init(&miniMedianFilter);
            dir = 0;
            dir_insert_sample = 0;
            insertedSamplesCounter = 0;

            ESP_LOGW(TAG, "GAP_FILL: done, median filters reset");
          }
        }

        if (p_payload != NULL) {
          #if 1
          int zero_write_streak = 0;
          do {
              size_t framesToBytes = (scSet.ch * (scSet.bits >> 3));
#if USE_SAMPLE_INSERTION
              uint32_t sampleSizeInBytes =
                  framesToBytes * INSERT_SAMPLES;

              if ((dir_insert_sample > 0) && (size >= sampleSizeInBytes)) {
                size -= sampleSizeInBytes;
                dir_insert_sample = 0;
              }
#endif
              if (s_write_cb) s_write_cb(p_payload, size, &written, s_write_ctx);

              if (written == 0) {
                if (player_stop_requested) {
                  ESP_LOGI(TAG, "STOP: aborting write mid-chunk, freeing");
                  free_pcm_chunk(chnk);
                  chnk = NULL;
                  dir = 0;
                  break;
                }
                if (++zero_write_streak >= 500) {
                  ESP_LOGW(TAG, "ZOMBIE_BAIL: write_cb returned 0 for %dms, dropping chunk", zero_write_streak);
                  free_pcm_chunk(chnk);
                  chnk = NULL;
                  dir = 0;
                  break;
                }
                vTaskDelay(pdMS_TO_TICKS(1));
                continue;
              }
              zero_write_streak = 0;

              samples_written += (written / framesToBytes);
              size -= written;
              p_payload += written;
              chunkStart += (1000000ll * (written / framesToBytes) / scSet.sr);

#if USE_SAMPLE_INSERTION
              if (dir_insert_sample < 0) {
                if (s_write_cb) s_write_cb(p_payload - sampleSizeInBytes, sampleSizeInBytes, &written, s_write_ctx);
                if (written != sampleSizeInBytes) {
                  ESP_LOGE(TAG, "i2s_playback_task:  write error %d", 1);
                }
                else {
                  samples_written += (written / framesToBytes);
                  dir_insert_sample = 0;
                  chunkStart += (1000000ll * (written / framesToBytes) / scSet.sr);
                }
              }
#endif
              if (size == 0) {
                if (fragment->nextFragment != NULL) {
                  fragment = fragment->nextFragment;
                  p_payload = fragment->payload;
                  size = fragment->size;

                } else {
                  prev_chunk_end_us = chunkStart;
                  free_pcm_chunk(chnk);
                  chnk = NULL;
                  dir = 0;

                  break;
                }
              }
            } while (1);

            outputBufferDacTime_us = 1000000ULL * i2sDmaBufMaxLen * i2sDmaBufCnt / scSet.sr;
          #else
          do {
            written = 0;

#if USE_SAMPLE_INSERTION
            uint32_t sampleSizeInBytes =
                (scSet.bits >> 3) * scSet.ch * INSERT_SAMPLES;

            if ((dir_insert_sample > 0) && (size >= sampleSizeInBytes)) {
              size -= sampleSizeInBytes;
            }
#endif
            int64_t alreadyWrittenTime_us = 0;
            size_t framesToBytes = (scSet.ch * (scSet.bits >> 3));
            while (size) {
              size_t i2sWriteLen;
              size_t tmpSize = i2sDmaBufMaxLen * framesToBytes;

              if (size >= tmpSize) {
                i2sWriteLen = i2sDmaBufMaxLen * framesToBytes - alreadyWritten;

                if (s_write_cb) s_write_cb(p_payload, i2sWriteLen, &written, s_write_ctx);

                alreadyWrittenTime_us =
                    1000000LL * (int64_t)(alreadyWritten / framesToBytes) /
                    (int64_t)scSet.sr;
                chunkStart += (dmaDescDuration_us - alreadyWrittenTime_us);

                alreadyWritten = 0;

                outputBufferDacTime_us =
                    1000000ULL * i2sDmaBufMaxLen * i2sDmaBufCnt / scSet.sr;
              } else {
                i2sWriteLen = size;

#if USE_SAMPLE_INSERTION
                size_t insertedSamplesWritten = 0;

                if (i2sWriteLen + sampleSizeInBytes <= i2sDmaBufMaxLen) {
                  if (dir_insert_sample < 0) {
                    if (s_write_cb) s_write_cb(p_payload, sampleSizeInBytes, &insertedSamplesWritten, s_write_ctx);
                    if (insertedSamplesWritten != sampleSizeInBytes) {
                      ESP_LOGE(TAG, "i2s_playback_task:  write error %d",
                               1);
                    }
                  }

                  dir_insert_sample = 0;
                }
#endif

                if (s_write_cb) s_write_cb(p_payload, i2sWriteLen, &written, s_write_ctx);

#if USE_SAMPLE_INSERTION
                alreadyWritten = written + insertedSamplesWritten;
#else
                alreadyWritten = written;
#endif
                alreadyWrittenTime_us =
                    1000000LL * (int64_t)(alreadyWritten / framesToBytes) /
                    (int64_t)scSet.sr;
                chunkStart += alreadyWrittenTime_us;

                outputBufferDacTime_us = (1000000ULL * i2sDmaBufMaxLen *
                                          (i2sDmaBufCnt - 1) / scSet.sr) +
                                         alreadyWrittenTime_us;
              }

              samples_written += (written / (scSet.ch * (scSet.bits / 8)));
              size -= written;
              p_payload += written;
            }

            dir = 0;

            if (size == 0) {
              if (fragment->nextFragment != NULL) {
                fragment = fragment->nextFragment;
                p_payload = fragment->payload;
                size = fragment->size;

              } else {
                free_pcm_chunk(chnk);
                chnk = NULL;
                dir = 0;

                break;
              }
            }
          } while (1);
          #endif
        } else {

          ESP_LOGW(TAG, "CHUNK_EMPTY: fragment alloc failed, writing %d bytes silence", size);
          written = 0;
          const size_t write_size = 4;
          uint8_t tmpBuf[write_size];

          memset(tmpBuf, 0, sizeof(tmpBuf));

          do {
            if (s_write_cb) s_write_cb(tmpBuf, write_size, &written, s_write_ctx);
            if (written != write_size) {
              ESP_LOGE(TAG, "i2s_playback_task: write error %d/%d", written,
                       size);
            }

            samples_written += (written / (scSet.ch * (scSet.bits / 8)));
            size -= written;
          } while (size);

          free_pcm_chunk(chnk);
          chnk = NULL;
        }

        if (server_now(&serverNow, &diff2Server) >= 0) {
          {
            int64_t now_us = esp_timer_get_time();

            int64_t samples_played = samples_written - i2sDmaBufCnt * i2sDmaBufMaxLen;
            #if 0

            double samples_expected = (target_play_local_us - playback_start_time_us) * ((float)scSet.sr / 1e6);
            double error_samples = samples_expected - samples_played;
            ESP_LOGI(TAG, "%0.2lf", error_samples);
            #endif

            if (tx_chan == NULL) outputBufferDacTime_us = 0;
            int64_t target_play_local_us = chunkStart - diff2Server + buf_us - outputBufferDacTime_us - clientDacLatency_us;

            int64_t actual_play_local_us = playback_start_time_us + (int64_t)((samples_played * 1000000ll) / (int64_t)scSet.sr);
            int64_t error_us = actual_play_local_us - target_play_local_us;

            age = error_us;
          }

          int64_t shortMedian, miniMedian;

          shortMedian = MEDIANFILTER_Insert(&shortMedianFilter, age);
          miniMedian = MEDIANFILTER_Insert(&miniMedianFilter, age);

          int msgWaiting = uxQueueMessagesWaiting(pcmChkQHdl);

          {
            static int prev_q = 0;
            if (msgWaiting < 20 && prev_q >= 20) {
              ESP_LOGW(TAG, "Q_LOW: q=%d (was %d) age=%lldus", msgWaiting, prev_q, age);
            }
            prev_q = msgWaiting;
          }

          if (abs_check_time > 0 && esp_timer_get_time() >= abs_check_time) {
            int64_t abs_age = serverNow - chunkStart - buf_us + clientDacLatency_us;
            int64_t delta = abs_age - age;
            ESP_LOGW(TAG, "ABS_CHECK_10S: abs=%lldus rel=%lldus delta=%lldus q=%d",
                     abs_age, age, delta, msgWaiting);
            abs_check_time = 0;
          }

          s_tele_age_us          = age;
          s_tele_short_median_us = shortMedian;
          s_tele_mini_median_us  = miniMedian;
          s_tele_queue_depth     = msgWaiting;
          s_tele_dir             = dir;
          s_tele_inserted        = insertedSamplesCounter;
          s_tele_initial_sync    = initialSync;

          {
            static int dbg_cnt = 0;
            if (++dbg_cnt >= 50) {
              dbg_cnt = 0;
              ESP_LOGI(TAG, "CTRL: age=%lldus sMed=%lldus mMed=%lldus q=%d dir=%d inserted=%ld",
                       age, shortMedian, miniMedian, msgWaiting, dir, insertedSamplesCounter);
            }
          }

          bool ext_resync = s_resync_requested;
          if (ext_resync) s_resync_requested = false;

          bool queue_drained = (msgWaiting == 0);
          if (queue_drained) {
            empty_q_count++;
          } else {
            empty_q_count = 0;
          }

          bool queue_drained_confirmed = (empty_q_count >= 3);

          bool median_error  = (MEDIANFILTER_isFull(&shortMedianFilter, 0) &&
                                ((shortMedian > hardResyncThreshold) ||
                                 (shortMedian < -hardResyncThreshold)));

          if (queue_drained_confirmed || median_error || ext_resync)
          {
            if (queue_drained_confirmed) {
              empty_q_count = 0;
            }

            if (chnk != NULL) {
              free_pcm_chunk(chnk);
              chnk = NULL;
            }

            wifi_ap_record_t ap;
            esp_wifi_sta_get_ap_info(&ap);

            const char *reason = ext_resync ? "MIXER_RST" : (queue_drained_confirmed ? "QDRAIN" : "TIMING");
            ESP_LOGW(TAG,
                     "RESYNCING HARD 2: reason=%s age=%lldus sMed=%lldus "
                     "latency=%lldus q=%d rssi=%d",
                     reason, age, shortMedian, diff2Server, msgWaiting, ap.rssi);

            my_gptimer_stop(gptimer);

            ESP_LOGW(TAG, "MUTE: HARD2 reason=%s age=%lldus q=%d",
                     reason, age, msgWaiting);
            audio_set_mute(true, "hard2");

            my_i2s_channel_disable(tx_chan);

            initialSync = 0;
            prev_chunk_end_us = 0;

            insertedSamplesCounter = 0;

            continue;
          }

#if USE_SAMPLE_INSERTION
          if ((enableControlLoop == true) &&
              (MEDIANFILTER_isFull(&shortMedianFilter, 0))) {
            if ((shortMedian < -shortOffset) && (miniMedian < -miniOffset) &&
                (age < -miniOffset)) {
              dir = -1;
              dir_insert_sample = -1;
              insertedSamplesCounter += INSERT_SAMPLES;
            } else if ((shortMedian > shortOffset) &&
                       (miniMedian > miniOffset) &&
                       (age > miniOffset)) {
              dir = 1;
              dir_insert_sample = 1;
              insertedSamplesCounter -= INSERT_SAMPLES;
            }
          }
#else
          if ((enableControlLoop == true) &&
              (MEDIANFILTER_isFull(&shortMedianFilter, 0))) {
            if ((shortMedian < -shortOffset) && (miniMedian < -miniOffset) &&
                (age < -miniOffset)) {
              dir = -1;
            } else if ((shortMedian > shortOffset) &&
                       (miniMedian > miniOffset) &&
                       (age > miniOffset)) {
              dir = 1;
            }

            adjust_apll(dir);
          }
#endif
           ESP_LOGD(TAG, "%d, %lldus, %lldus, %lldus, q:%d, %lld, %lld", dir,
                   age, shortMedian, miniMedian,
                   uxQueueMessagesWaiting(pcmChkQHdl), insertedSamplesCounter,
                   chunkDuration_us);

        } else {
          ESP_LOGW(TAG, "CTRL_DROP: server_now() failed, chunk discarded q=%d",
                   pcmChkQHdl ? (int)uxQueueMessagesWaiting(pcmChkQHdl) : -1);

          if (chnk != NULL) {
            free_pcm_chunk(chnk);
            chnk = NULL;
          }

          vTaskDelay(pdMS_TO_TICKS(1));

          continue;
        }
      }
    } else {
      int64_t sec, msec, usec;

      sec = diff2Server / 1000000;
      usec = diff2Server - sec * 1000000;
      msec = usec / 1000;
      usec = usec % 1000;

      if (pcmChkQHdl != NULL) {
        ESP_LOGV(TAG,
                 "Couldn't get PCM chunk, recv: messages waiting %d, "
                 "diff2Server: %llds, %lld.%lldms",
                 uxQueueMessagesWaiting(pcmChkQHdl), sec, msec, usec);
      }

      dir = 0;
      initialSync = 0;
      prev_chunk_end_us = 0;
      ESP_LOGW(TAG, "MUTE: PLAYER_STOP");
      audio_set_mute(true, "stop");
      audio_dac_enable(false);
      my_i2s_channel_disable(tx_chan);

      break;
    }
  }
  ret = 0;

  xSemaphoreTake(snapcastSettingsMux, portMAX_DELAY);

  vQueueDelete(snapcastSettingQueueHandle);
  snapcastSettingQueueHandle = NULL;
  xSemaphoreGive(snapcastSettingsMux);

#if CONFIG_PM_ENABLE
  esp_pm_lock_release(player_pm_lock_handle);
#endif

  ret = destroy_pcm_queue(&pcmChkQHdl);

  tg0_timer_deinit();
  playerstarted = false;
  ESP_LOGI(TAG, "stop player done");
  playerTaskHandle = NULL;
  vTaskDelete(NULL);
}

