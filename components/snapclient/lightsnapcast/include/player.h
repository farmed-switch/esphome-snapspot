#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "driver/i2s_std.h"
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "sdkconfig.h"
#include "snapcast.h"

#ifdef __cplusplus
extern "C" {
#endif

#define USE_TIMEFILTER  CONFIG_SNAPCLIENT_USE_TIMEFILTER

#define CHNK_CTRL_CNT 2

#define LATENCY_TIME_FILTER_FULL 29

#define LATENCY_MEDIAN_AVG_DIVISOR 0

#define LATENCY_MEDIAN_FILTER_LEN 199
#define LATENCY_MEDIAN_FILTER_FULL 19

#define SHORT_BUFFER_LEN 99
#define MINI_BUFFER_LEN 19

typedef struct pcm_chunk_fragment pcm_chunk_fragment_t;
struct pcm_chunk_fragment {
  size_t size;
  char *payload;
  pcm_chunk_fragment_t *nextFragment;
};

typedef struct pcmData {
  tv_t timestamp;
  size_t totalSize;
  pcm_chunk_fragment_t *fragment;
  uint32_t caps;
} pcm_chunk_message_t;

typedef enum codec_type_e { NONE = 0, PCM, FLAC, OGG, OPUS } codec_type_t;

typedef struct snapcastSetting_s {
  uint32_t buf_ms;
  uint32_t chkInFrames;
  int32_t cDacLat_ms;

  codec_type_t codec;
  int32_t sr;
  uint8_t ch;
  i2s_data_bit_width_t bits;

  bool muted;
  uint32_t volume;

  char *pcmBuf;
  uint32_t pcmBufSize;
} snapcastSetting_t;

typedef void (*player_write_cb_t)(const void *buf, size_t bytes,
                                   size_t *written, void *ctx);

int init_player(player_write_cb_t write_cb, void *ctx);
int deinit_player(void);
bool is_player_running(void);
void player_request_stop(void);
int start_player(snapcastSetting_t *setting);

int32_t allocate_pcm_chunk_memory(pcm_chunk_message_t **pcmChunk, size_t bytes);
int32_t insert_pcm_chunk(pcm_chunk_message_t *pcmChunk);

int8_t free_pcm_chunk(pcm_chunk_message_t *pcmChunk);

#if USE_TIMEFILTER
int32_t player_latency_insert(int64_t newValue, int64_t max_error, int64_t time_added);
#else
int32_t player_latency_insert(int64_t newValue);
#endif

int32_t get_diff_to_server(int64_t *tDiff, int64_t now);
int32_t latency_buffer_full(bool *is_full);

int32_t player_send_snapcast_setting(snapcastSetting_t *setting);
int8_t player_get_snapcast_settings(snapcastSetting_t *setting);

int32_t reset_latency_buffer(void);

int32_t server_now(int64_t *sNow, int64_t *diff2Server);

int32_t pcm_chunk_queue_msg_waiting(void);

typedef struct {
  int64_t age_us;
  int64_t short_median_us;
  int64_t mini_median_us;
  int      queue_depth;
  int      dir;
  int64_t  inserted_samples;
  int      initial_sync;
} player_telemetry_t;

void player_get_telemetry(player_telemetry_t *out);

double player_get_latency_drift(void);

void player_request_resync(void);

#ifdef __cplusplus
}
#endif
#endif
