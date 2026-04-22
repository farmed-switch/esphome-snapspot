#pragma once

#ifdef USE_ESP32
#ifndef USE_I2S_LEGACY
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esphome/core/defines.h"

namespace esphome {
namespace snapclient {

static const int HTTP_TASK_PRIORITY = 11;
static const int HTTP_TASK_CORE_ID = tskNO_AFFINITY;

typedef struct audioDACdata_s {
  bool mute;
  int volume;
} audioDACdata_t;

void http_get_task(void *pvParameters);
void init_snapcast(QueueHandle_t audioQHdl, const char *name, const char *host, uint16_t port);
void decoder_request_stop(void);
void decoder_clear_stop(void);
bool decoder_is_stopped(void);
void decoder_get_counters(uint32_t *recv, uint32_t *ok, uint32_t *drop);
void decoder_get_diag(int64_t *max_loop, int64_t *max_recv, int64_t *tsync_us, uint32_t *tout_cnt, uint32_t *recv_bytes);
void decoder_get_tcp_pcb_count(int *active, int *tw);
int decoder_get_http_core(void);
extern "C" void audio_set_mute(bool mute, const char *tag);
void audio_set_volume(int volume);

}
}
#endif
#endif
