
#ifdef USE_ESP32
#ifndef USE_I2S_LEGACY

#include "decoder.h"
#include "snapclient.h"
#include "esphome/core/log.h"
#include "esphome/components/audio/audio.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_netif.h"
#include "lwip/api.h"
#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/tcp.h"
#include "lwip/tcpip.h"
#include "lwip/priv/tcp_priv.h"
#include "mdns.h"

#include "driver/i2s_std.h"
#if CONFIG_USE_DSP_PROCESSOR
#include "dsp_processor.h"
#endif

#ifdef USE_SHARED_AUDIO_EQ
#include "esphome/components/snapspot/shared_audio_eq.h"
#endif

#include "opus.h"

#include "FLAC/stream_decoder.h"
#include "player.h"
#include "snapcast.h"

namespace esphome {
namespace snapclient {

static bool isCachedChunk = false;
static uint32_t cachedBlocks = 0;

static FLAC__StreamDecoder *flacDecoder = NULL;

const char *VERSION_STRING = "0.0.3";

void time_sync_msg_cb(void *args);
static FLAC__StreamDecoderReadStatus read_callback(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[],
                                                   size_t *bytes, void *client_data);
static FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame,
                                                     const FLAC__int32 *const buffer[], void *client_data);
static void metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata,
                              void *client_data);
static void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status,
                           void *client_data);

static const int FAST_SYNC_LATENCY_BUF = 10000;
static const int NORMAL_SYNC_LATENCY_BUF = 1000000;

struct timeval tdif, tavg;

#define SNAPCAST_SERVER_USE_MDNS CONFIG_SNAPSERVER_USE_MDNS
char *SNAPCAST_SERVER_HOST;
uint16_t SNAPCAST_SERVER_PORT;
char *SNAPCAST_CLIENT_NAME;
#define SNAPCAST_USE_SOFT_VOL CONFIG_SNAPCLIENT_USE_SOFT_VOL

SemaphoreHandle_t timeSyncSemaphoreHandle = NULL;

#if CONFIG_USE_DSP_PROCESSOR
#if CONFIG_SNAPCLIENT_DSP_FLOW_STEREO
dspFlows_t dspFlow = dspfStereo;
#endif
#if CONFIG_SNAPCLIENT_DSP_FLOW_BASSBOOST
dspFlows_t dspFlow = dspfBassBoost;
#endif
#if CONFIG_SNAPCLIENT_DSP_FLOW_BIAMP
dspFlows_t dspFlow = dspfBiamp;
#endif
#if CONFIG_SNAPCLIENT_DSP_FLOW_BASS_TREBLE_EQ
dspFlows_t dspFlow = dspfEQBassTreble;
#endif
#endif

audioDACdata_t audioDAC_data;
static QueueHandle_t audioDACQHdl = NULL;
SemaphoreHandle_t audioDACSemaphore = NULL;

typedef struct decoderData_s {
  uint32_t type;

  uint8_t *inData;
  tv_t timestamp;
  uint8_t *outData;
  uint32_t bytes;
} decoderData_t;

static char base_message_serialized[BASE_MESSAGE_SIZE];

struct netconn *lwipNetconn;

static volatile uint32_t s_dec_recv = 0;
static volatile uint32_t s_dec_ok = 0;
static volatile uint32_t s_dec_drop = 0;
static volatile int s_http_core = -1;

static volatile int64_t s_max_loop_us = 0;
static volatile int64_t s_max_recv_us = 0;
static volatile int64_t s_tsync_write_us = 0;
static volatile uint32_t s_recv_timeout_cnt = 0;
static volatile uint32_t s_recv_bytes = 0;

void decoder_get_counters(uint32_t *recv, uint32_t *ok, uint32_t *drop) {
  *recv = s_dec_recv;
  *ok   = s_dec_ok;
  *drop = s_dec_drop;
}

void decoder_get_diag(int64_t *max_loop, int64_t *max_recv, int64_t *tsync_us, uint32_t *tout_cnt, uint32_t *recv_bytes) {
  *max_loop = s_max_loop_us;
  *max_recv = s_max_recv_us;
  *tsync_us = s_tsync_write_us;
  *tout_cnt = s_recv_timeout_cnt;
  *recv_bytes = s_recv_bytes;
  s_max_loop_us = 0;
  s_max_recv_us = 0;
}

int decoder_get_http_core(void) {
  return s_http_core;
}

void decoder_get_tcp_pcb_count(int *active, int *tw) {
  int a = 0, t = 0;
  LOCK_TCPIP_CORE();
  for (struct tcp_pcb *pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next) a++;
  for (struct tcp_pcb *pcb = tcp_tw_pcbs; pcb != NULL; pcb = pcb->next) t++;
  UNLOCK_TCPIP_CORE();
  *active = a;
  *tw = t;
}

static volatile bool decoder_stop_requested = false;

void decoder_request_stop(void) {
  decoder_stop_requested = true;
  if (lwipNetconn) {
    netconn_close(lwipNetconn);
  }
}

void decoder_clear_stop(void) {
  decoder_stop_requested = false;
}

bool decoder_is_stopped(void) {
  return decoder_stop_requested;
}

static int id_counter = 0;

static OpusDecoder *opusDecoder = NULL;

static decoderData_t decoderChunk = {
    .type = SNAPCAST_MESSAGE_INVALID,
    .inData = NULL,
    .timestamp = {0, 0},
    .outData = NULL,
    .bytes = 0,
};

static decoderData_t pcmChunk = {
    .type = SNAPCAST_MESSAGE_INVALID,
    .inData = NULL,
    .timestamp = {0, 0},
    .outData = NULL,
    .bytes = 0,
};

void time_sync_msg_cb(void *args) {
  base_message_t base_message_tx;

  int64_t now;

  int rc1;

  uint8_t *p_pkt = (uint8_t *) malloc(BASE_MESSAGE_SIZE + TIME_MESSAGE_SIZE);
  if (p_pkt == NULL) {
    ESP_LOGW(TAG, "%s: Failed to get memory for time sync message. Skipping this round.", __func__);

    return;
  }

  memset(p_pkt, 0, BASE_MESSAGE_SIZE + TIME_MESSAGE_SIZE);

  base_message_tx.type = SNAPCAST_MESSAGE_TIME;
  base_message_tx.id = id_counter++;
  base_message_tx.refersTo = 0;
  base_message_tx.received.sec = 0;
  base_message_tx.received.usec = 0;
  now = esp_timer_get_time();
  base_message_tx.sent.sec = now / 1000000;
  base_message_tx.sent.usec = now - base_message_tx.sent.sec * 1000000;
  base_message_tx.size = TIME_MESSAGE_SIZE;
  rc1 = base_message_serialize(&base_message_tx, (char *) &p_pkt[0], BASE_MESSAGE_SIZE);
  if (rc1) {
    ESP_LOGE(TAG, "Failed to serialize base message for time");
    free(p_pkt);
    return;
  }

  int64_t tsync_t0 = esp_timer_get_time();
  rc1 = netconn_write(lwipNetconn, p_pkt, BASE_MESSAGE_SIZE + TIME_MESSAGE_SIZE, NETCONN_COPY);
  s_tsync_write_us = esp_timer_get_time() - tsync_t0;
  if (rc1 != ERR_OK) {
    ESP_LOGW(TAG, "error writing timesync msg");
    free(p_pkt);
    return;
  }

  free(p_pkt);

}

static FLAC__StreamDecoderReadStatus read_callback(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[],
                                                   size_t *bytes, void *client_data) {
  snapcastSetting_t *scSet = (snapcastSetting_t *) client_data;

  (void) scSet;

  if (decoderChunk.inData) {

    if (decoderChunk.bytes <= 0) {

      return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }

    isCachedChunk = false;

    if (decoderChunk.bytes <= *bytes) {
      memcpy(buffer, decoderChunk.inData, decoderChunk.bytes);
      *bytes = decoderChunk.bytes;

      free(decoderChunk.inData);
      decoderChunk.inData = NULL;
      decoderChunk.bytes = 0;
    } else {
      memcpy(buffer, decoderChunk.inData, *bytes);

      memmove(decoderChunk.inData, decoderChunk.inData + *bytes, decoderChunk.bytes - *bytes);
      decoderChunk.bytes -= *bytes;
      decoderChunk.inData = (uint8_t *) realloc(decoderChunk.inData, decoderChunk.bytes);

    }

    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
  } else {
    return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
  }
}

static FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame,
                                                     const FLAC__int32 *const buffer[], void *client_data) {
  size_t i;
  snapcastSetting_t *scSet = (snapcastSetting_t *) client_data;

  size_t bytes = frame->header.blocksize * frame->header.channels * frame->header.bits_per_sample / 8;

  (void) decoder;

  if (isCachedChunk) {
    cachedBlocks += frame->header.blocksize;
  }

  if (frame->header.channels != scSet->ch) {
    ESP_LOGE(TAG,
             "ERROR: frame header reports different channel count %ld than "
             "previous metadata block %d",
             frame->header.channels, scSet->ch);
    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
  }
  if (frame->header.bits_per_sample != scSet->bits) {
    ESP_LOGE(TAG,
             "ERROR: frame header reports different bps %ld than previous "
             "metadata block %d",
             frame->header.bits_per_sample, scSet->bits);
    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
  }
  if (buffer[0] == NULL) {
    ESP_LOGE(TAG, "ERROR: buffer [0] is NULL");
    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
  }
  if (buffer[1] == NULL) {
    ESP_LOGE(TAG, "ERROR: buffer [1] is NULL");
    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
  }

  pcmChunk.outData = (uint8_t *) realloc(pcmChunk.outData, pcmChunk.bytes + bytes);
  if (!pcmChunk.outData) {
    ESP_LOGE(TAG, "%s, failed to allocate PCM chunk payload", __func__);
    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
  }

  for (i = 0; i < frame->header.blocksize; i++) {

    pcmChunk.outData[pcmChunk.bytes + 4 * i] = (uint8_t) (buffer[0][i]);
    pcmChunk.outData[pcmChunk.bytes + 4 * i + 1] = (uint8_t) (buffer[0][i] >> 8);
    pcmChunk.outData[pcmChunk.bytes + 4 * i + 2] = (uint8_t) (buffer[1][i]);
    pcmChunk.outData[pcmChunk.bytes + 4 * i + 3] = (uint8_t) (buffer[1][i] >> 8);
  }

  pcmChunk.bytes += bytes;

  scSet->chkInFrames = frame->header.blocksize;

  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data) {
  snapcastSetting_t *scSet = (snapcastSetting_t *) client_data;

  (void) decoder;

  if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {

    scSet->sr = metadata->data.stream_info.sample_rate;
    scSet->ch = metadata->data.stream_info.channels;
    scSet->bits = (i2s_data_bit_width_t) metadata->data.stream_info.bits_per_sample;

    ESP_LOGI(TAG, "fLaC sampleformat: %ld:%d:%d", scSet->sr, scSet->bits, scSet->ch);

  }
}

void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data) {
  (void) decoder, (void) client_data;

  ESP_LOGE(TAG, "Got error callback: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
}

void init_snapcast(QueueHandle_t audioQHdl, const char *name, const char *host, uint16_t port) {
  audioDACQHdl = audioQHdl;
  audioDACSemaphore = xSemaphoreCreateMutex();
  audioDAC_data.mute = true;
  audioDAC_data.volume = 100;
  SNAPCAST_CLIENT_NAME = (char *) name;
  SNAPCAST_SERVER_HOST = (char *) host;
  SNAPCAST_SERVER_PORT = port;
}

extern "C" void snapclient_fr_mute(int on, const char *caller);

extern "C" void audio_set_mute(bool mute, const char *tag) {
  xSemaphoreTake(audioDACSemaphore, portMAX_DELAY);
  if (mute != audioDAC_data.mute) {
    ESP_LOGW("snapclient", "MUTE_CHG: %s caller=%s core=%d",
             mute ? "ON" : "OFF", tag ? tag : "?", xPortGetCoreID());
    audioDAC_data.mute = mute;
    xQueueOverwrite(audioDACQHdl, &audioDAC_data);
    snapclient_fr_mute(mute ? 1 : 0, tag);
  }
  xSemaphoreGive(audioDACSemaphore);
}

void audio_set_volume(int volume) {
  xSemaphoreTake(audioDACSemaphore, portMAX_DELAY);
  if (volume != audioDAC_data.volume) {
    audioDAC_data.volume = volume;
    xQueueOverwrite(audioDACQHdl, &audioDAC_data);
  }
  xSemaphoreGive(audioDACSemaphore);
}

void http_get_task(void *pvParameters) {
  auto *snap_component = static_cast<SnapClientComponent *>(pvParameters);
  char *start;
  base_message_t base_message_rx;
  hello_message_t hello_message;
  wire_chunk_message_t wire_chnk = {{0, 0}, 0, NULL};
  char *hello_message_serialized = NULL;
  int result;
  int64_t now, trx, tdif, ttx;
  time_message_t time_message_rx = {{0, 0}};
  int64_t tmpDiffToServer;
  int64_t lastTimeSync = 0;
  esp_err_t err = 0;
  server_settings_message_t server_settings_message;
  bool received_header = false;
  mdns_result_t *r;
  codec_type_t codec = NONE;
  snapcastSetting_t scSet;
  pcm_chunk_message_t *pcmData = NULL;
  ip_addr_t remote_ip;
  uint16_t remotePort = 0;
  int rc1 = ERR_OK, rc2 = ERR_OK;
  struct netbuf *firstNetBuf = NULL;
  uint16_t len;
  uint64_t timeout = FAST_SYNC_LATENCY_BUF;
  char *codecString = NULL;
  char *codecPayload = NULL;
  char *serverSettingsString = NULL;

#if CONFIG_SNAPCLIENT_USE_MDNS
  ESP_LOGI(TAG, "Enable mdns");
  mdns_init();
#endif

  while (!esp_netif_get_default_netif() && !decoder_stop_requested) {
    ESP_LOGI(TAG, "Waiting for network interface...");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  while (!decoder_stop_requested) {

    {
      received_header = false;

      timeout = FAST_SYNC_LATENCY_BUF;

      if (opusDecoder != NULL) {
        opus_decoder_destroy(opusDecoder);
        opusDecoder = NULL;
      }

      if (flacDecoder != NULL) {
        FLAC__stream_decoder_finish(flacDecoder);
        FLAC__stream_decoder_delete(flacDecoder);
        flacDecoder = NULL;
      }

      if (decoderChunk.inData) {
        free(decoderChunk.inData);
        decoderChunk.inData = NULL;
      }

      if (decoderChunk.outData) {
        free(decoderChunk.outData);
        decoderChunk.outData = NULL;
      }

      if (codecString) {
        free(codecString);
        codecString = NULL;
      }

      if (codecPayload) {
        free(codecPayload);
        codecPayload = NULL;
      }

      if (codecPayload) {
        free(serverSettingsString);
        serverSettingsString = NULL;
      }
    }

#if SNAPCAST_SERVER_USE_MDNS

    r = NULL;
    err = 0;
    while (!r || err) {
      ESP_LOGI(TAG, "Lookup snapcast service on network");
      int64_t mdns_t0 = esp_timer_get_time();
      esp_err_t err = mdns_query_ptr("_snapcast", "_tcp", 3000, 20, &r);
      int64_t mdns_ms = (esp_timer_get_time() - mdns_t0) / 1000;
      ESP_LOGI(TAG, "mDNS query took %lldms", (long long)mdns_ms);
      if (err) {
        ESP_LOGE(TAG, "Query Failed");
        vTaskDelay(pdMS_TO_TICKS(1000));
      }

      if (!r) {
        ESP_LOGW(TAG, "No results found!");
        vTaskDelay(pdMS_TO_TICKS(1000));
      }
    }

    mdns_ip_addr_t *a = r->addr;
    if (a) {
      ip_addr_copy(remote_ip, (a->addr));
      remote_ip.type = a->addr.type;
      remotePort = r->port;
      ESP_LOGI(TAG, "Found %s:%d", ipaddr_ntoa(&remote_ip), remotePort);

      mdns_query_results_free(r);
    } else {
      mdns_query_results_free(r);

      ESP_LOGW(TAG, "No IP found in MDNS query");

      continue;
    }
#else

    struct sockaddr_in servaddr;

    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, SNAPCAST_SERVER_HOST, &(servaddr.sin_addr.s_addr));
    servaddr.sin_port = htons(SNAPCAST_SERVER_PORT);

#if USE_NETWORK_IPV6
    inet_pton(AF_INET, SNAPCAST_SERVER_HOST, &(remote_ip.u_addr.ip4.addr));
    remote_ip.type = IPADDR_TYPE_V4;
#else
    if (!(inet_pton(AF_INET, SNAPCAST_SERVER_HOST, &remote_ip) == 1)) {
      ESP_LOGE(TAG, "invalid snapcast server ip");
      return;
    }
#endif
    remotePort = SNAPCAST_SERVER_PORT;

    ESP_LOGI(TAG, "try connecting to static configuration %s:%d", ipaddr_ntoa(&remote_ip), remotePort);
#endif

    if (lwipNetconn != NULL) {
      netconn_delete(lwipNetconn);
      lwipNetconn = NULL;
    }

    lwipNetconn = netconn_new(NETCONN_TCP);
    if (lwipNetconn == NULL) {
      ESP_LOGE(TAG, "can't create netconn");

      continue;
    }

    rc1 = netconn_bind(lwipNetconn, (const ip_addr_t *) IPADDR_ANY, 0);
    if (rc1 != ERR_OK) {
      ESP_LOGE(TAG, "can't bind local IP");
    }

    rc2 = netconn_connect(lwipNetconn, &remote_ip, remotePort);
    if (rc2 != ERR_OK) {
      ESP_LOGE(TAG, "can't connect to remote %s:%d, err %d", ipaddr_ntoa(&remote_ip), remotePort, rc2);
    }

    if (rc1 != ERR_OK || rc2 != ERR_OK) {
      netconn_close(lwipNetconn);
      netconn_delete(lwipNetconn);
      lwipNetconn = NULL;

      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }

    ESP_LOGI(TAG, "netconn connected");

    netconn_set_recvtimeout(lwipNetconn, timeout / 1000);

    if (reset_latency_buffer() < 0) {
      ESP_LOGE(TAG, "reset_diff_buffer: couldn't reset median filter long. STOP");
      return;
    }

    char mac_address[18];
    uint8_t base_mac[6];

#if CONFIG_SNAPCLIENT_USE_INTERNAL_ETHERNET || CONFIG_SNAPCLIENT_USE_SPI_ETHERNET
    esp_read_mac(base_mac, ESP_MAC_ETH);
#else
    esp_read_mac(base_mac, ESP_MAC_WIFI_STA);
#endif
    sprintf(mac_address, "%02X:%02X:%02X:%02X:%02X:%02X", base_mac[0], base_mac[1], base_mac[2], base_mac[3],
            base_mac[4], base_mac[5]);

    now = esp_timer_get_time();

    base_message_rx.type = SNAPCAST_MESSAGE_HELLO;
    base_message_rx.id = id_counter++;
    base_message_rx.refersTo = 0x0000;
    base_message_rx.sent.sec = now / 1000000;
    base_message_rx.sent.usec = now - base_message_rx.sent.sec * 1000000;
    base_message_rx.received.sec = 0;
    base_message_rx.received.usec = 0;
    base_message_rx.size = 0x00000000;

    hello_message.mac = mac_address;
    hello_message.hostname = SNAPCAST_CLIENT_NAME;
    hello_message.version = (char *) VERSION_STRING;
    hello_message.client_name = "libsnapcast";
    hello_message.os = "esp32";
    hello_message.arch = "xtensa";
    hello_message.instance = 1;
    hello_message.id = mac_address;
    hello_message.protocol_version = 2;

    if (hello_message_serialized == NULL) {
      hello_message_serialized = hello_message_serialize(&hello_message, (size_t *) &(base_message_rx.size));
      if (!hello_message_serialized) {
        ESP_LOGE(TAG, "Failed to serialize hello message");
        return;
      }
    }

    result = base_message_serialize(&base_message_rx, base_message_serialized, BASE_MESSAGE_SIZE);
    if (result) {
      ESP_LOGE(TAG, "Failed to serialize base message");
      return;
    }

    rc1 = netconn_write(lwipNetconn, base_message_serialized, BASE_MESSAGE_SIZE, NETCONN_COPY);
    if (rc1 != ERR_OK) {
      ESP_LOGE(TAG, "netconn failed to send base message");

      continue;
    }
    rc1 = netconn_write(lwipNetconn, hello_message_serialized, base_message_rx.size, NETCONN_COPY);
    if (rc1 != ERR_OK) {
      ESP_LOGE(TAG, "netconn failed to send hello message");

      continue;
    }

    ESP_LOGI(TAG, "netconn sent hello message");

    free(hello_message_serialized);
    hello_message_serialized = NULL;

    scSet.buf_ms = 500;
    scSet.codec = NONE;
    scSet.bits = (i2s_data_bit_width_t) 16;
    scSet.ch = 2;
    scSet.sr = 44100;
    scSet.chkInFrames = 0;
    scSet.volume = 0;
    scSet.muted = true;

    uint64_t startTime, endTime;

    size_t typedMsgCurrentPos = 0;
    uint32_t typedMsgLen = 0;
    uint32_t offset = 0;
    uint32_t payloadOffset = 0;
    uint32_t tmpData = 0;
    int32_t payloadDataShift = 0;

    static const uint8_t BASE_MESSAGE_STATE = 0;
    static const uint8_t TYPED_MESSAGE_STATE = 1;

    uint32_t state = BASE_MESSAGE_STATE;
    uint32_t internalState = 0;

    firstNetBuf = NULL;

    while (!decoder_stop_requested) {
      int64_t recv_start_us = esp_timer_get_time();
      rc2 = netconn_recv(lwipNetconn, &firstNetBuf);
      int64_t recv_elapsed_us = esp_timer_get_time() - recv_start_us;
      int64_t recv_dur_ms = recv_elapsed_us / 1000;
      if (recv_elapsed_us > s_max_recv_us) s_max_recv_us = recv_elapsed_us;
      if (recv_dur_ms > 1500) {
        ESP_LOGW(TAG, "TCP_SLOW: netconn_recv blocked %lldms core=%d", recv_dur_ms, xPortGetCoreID());
      }
      {
        int cur = xPortGetCoreID();
        if (cur != s_http_core) {
          ESP_LOGV(TAG, "HTTP_CORE: http_get_task moved %d -> %d", s_http_core, cur);
          s_http_core = cur;
        }
      }
      s_dec_recv++;
      if (rc2 != ERR_OK) {
        if (rc2 == ERR_CONN) {
          netconn_close(lwipNetconn);

          break;
        }

        if (rc2 == ERR_TIMEOUT) {
          s_recv_timeout_cnt++;
          if (firstNetBuf != NULL) { netbuf_delete(firstNetBuf); firstNetBuf = NULL; }

          continue;
        }

        ESP_LOGW(TAG, "TCP_ERR: netconn_recv err=%d", rc2);

        if (firstNetBuf != NULL) {
          netbuf_delete(firstNetBuf);

          firstNetBuf = NULL;
        }
        continue;
      }

      s_recv_bytes += netbuf_len(firstNetBuf);

      now = esp_timer_get_time();
      if (received_header && (now - lastTimeSync) >= (int64_t)timeout && recv_elapsed_us < 100000) {
        time_sync_msg_cb(NULL);
        lastTimeSync = now;
      }

      netbuf_first(firstNetBuf);
      do {

        rc1 = netbuf_data(firstNetBuf, (void **) &start, &len);
        if (rc1 == ERR_OK) {

        } else {
          ESP_LOGE(TAG, "netconn rx, couldn't get data");

          continue;
        }

        while (len > 0) {
          rc1 = ERR_OK;

          switch (state) {

            case BASE_MESSAGE_STATE: {
              switch (internalState) {
                case 0:
                  base_message_rx.type = *start & 0xFF;
                  internalState++;
                  break;

                case 1:
                  base_message_rx.type |= (*start & 0xFF) << 8;
                  internalState++;
                  break;

                case 2:
                  base_message_rx.id = *start & 0xFF;
                  internalState++;
                  break;

                case 3:
                  base_message_rx.id |= (*start & 0xFF) << 8;
                  internalState++;
                  break;

                case 4:
                  base_message_rx.refersTo = *start & 0xFF;
                  internalState++;
                  break;

                case 5:
                  base_message_rx.refersTo |= (*start & 0xFF) << 8;
                  internalState++;
                  break;

                case 6:
                  base_message_rx.sent.sec = *start & 0xFF;
                  internalState++;
                  break;

                case 7:
                  base_message_rx.sent.sec |= (*start & 0xFF) << 8;
                  internalState++;
                  break;

                case 8:
                  base_message_rx.sent.sec |= (*start & 0xFF) << 16;
                  internalState++;
                  break;

                case 9:
                  base_message_rx.sent.sec |= (*start & 0xFF) << 24;
                  internalState++;
                  break;

                case 10:
                  base_message_rx.sent.usec = *start & 0xFF;
                  internalState++;
                  break;

                case 11:
                  base_message_rx.sent.usec |= (*start & 0xFF) << 8;
                  internalState++;
                  break;

                case 12:
                  base_message_rx.sent.usec |= (*start & 0xFF) << 16;
                  internalState++;
                  break;

                case 13:
                  base_message_rx.sent.usec |= (*start & 0xFF) << 24;
                  internalState++;
                  break;

                case 14:
                  base_message_rx.received.sec = *start & 0xFF;
                  internalState++;
                  break;

                case 15:
                  base_message_rx.received.sec |= (*start & 0xFF) << 8;
                  internalState++;
                  break;

                case 16:
                  base_message_rx.received.sec |= (*start & 0xFF) << 16;
                  internalState++;
                  break;

                case 17:
                  base_message_rx.received.sec |= (*start & 0xFF) << 24;
                  internalState++;
                  break;

                case 18:
                  base_message_rx.received.usec = *start & 0xFF;
                  internalState++;
                  break;

                case 19:
                  base_message_rx.received.usec |= (*start & 0xFF) << 8;
                  internalState++;
                  break;

                case 20:
                  base_message_rx.received.usec |= (*start & 0xFF) << 16;
                  internalState++;
                  break;

                case 21:
                  base_message_rx.received.usec |= (*start & 0xFF) << 24;
                  internalState++;
                  break;

                case 22:
                  base_message_rx.size = *start & 0xFF;
                  internalState++;
                  break;

                case 23:
                  base_message_rx.size |= (*start & 0xFF) << 8;
                  internalState++;
                  break;

                case 24:
                  base_message_rx.size |= (*start & 0xFF) << 16;
                  internalState++;
                  break;

                case 25:
                  base_message_rx.size |= (*start & 0xFF) << 24;
                  internalState = 0;

                  now = esp_timer_get_time();

                  base_message_rx.received.sec = now / 1000000;
                  base_message_rx.received.usec = now - base_message_rx.received.sec * 1000000;

                  typedMsgCurrentPos = 0;

                  state = TYPED_MESSAGE_STATE;
                  break;
              }

              len--;
              start++;

              break;
            }

            case TYPED_MESSAGE_STATE: {
              switch (base_message_rx.type) {
                case SNAPCAST_MESSAGE_WIRE_CHUNK: {
                  switch (internalState) {
                    case 0: {
                      wire_chnk.timestamp.sec = *start & 0xFF;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 1: {
                      wire_chnk.timestamp.sec |= (*start & 0xFF) << 8;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 2: {
                      wire_chnk.timestamp.sec |= (*start & 0xFF) << 16;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 3: {
                      wire_chnk.timestamp.sec |= (*start & 0xFF) << 24;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 4: {
                      wire_chnk.timestamp.usec = (*start & 0xFF);

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 5: {
                      wire_chnk.timestamp.usec |= (*start & 0xFF) << 8;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 6: {
                      wire_chnk.timestamp.usec |= (*start & 0xFF) << 16;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 7: {
                      wire_chnk.timestamp.usec |= (*start & 0xFF) << 24;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 8: {
                      wire_chnk.size = (*start & 0xFF);

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 9: {
                      wire_chnk.size |= (*start & 0xFF) << 8;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 10: {
                      wire_chnk.size |= (*start & 0xFF) << 16;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 11: {
                      wire_chnk.size |= (*start & 0xFF) << 24;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      decoderChunk.bytes = wire_chnk.size;

                      if (decoderChunk.bytes > 131072) {
                        ESP_LOGW(TAG, "DECODE_DROP: chunk size %d > 128KB, corrupt? dropping",
                                 decoderChunk.bytes);
                        s_dec_drop++;
                        decoderChunk.bytes = 0;
                        decoderChunk.inData = NULL;

                        size_t skip = base_message_rx.size - typedMsgCurrentPos;
                        if (skip <= len) {
                          start += skip;
                          len -= skip;
                          typedMsgCurrentPos += skip;
                        } else {
                          typedMsgCurrentPos += len;
                          len = 0;
                        }
                        state = BASE_MESSAGE_STATE;
                        internalState = 0;
                        break;
                      }

                      {
                        int alloc_retries = 0;
                        while (!decoderChunk.inData) {
                          decoderChunk.inData = (uint8_t *) malloc(decoderChunk.bytes);
                          if (!decoderChunk.inData) {
                            if (++alloc_retries > 3) {
                              ESP_LOGW(TAG, "DECODE_DROP: malloc(%d) failed after %d retries, dropping chunk",
                                       decoderChunk.bytes, alloc_retries);
                              s_dec_drop++;
                              break;
                            }
                            vTaskDelay(pdMS_TO_TICKS(1));
                          }
                        }
                        if (!decoderChunk.inData) {
                          decoderChunk.bytes = 0;

                          size_t skip = base_message_rx.size - typedMsgCurrentPos;
                          if (skip <= len) {
                            start += skip;
                            len -= skip;
                            typedMsgCurrentPos += skip;
                          } else {
                            typedMsgCurrentPos += len;
                            len = 0;
                          }
                          state = BASE_MESSAGE_STATE;
                          internalState = 0;
                          break;
                        }
                      }

                      payloadOffset = 0;

#if 0
                       ESP_LOGI(TAG, "chunk with size: %u, at time %ld.%ld",
                                             wire_chnk.size,
                                             wire_chnk.timestamp.sec,
                                             wire_chnk.timestamp.usec);
#endif

                      if (len == 0) {
                        break;
                      }
                    }

                    case 12: {
                      size_t tmp_size;

                      if ((base_message_rx.size - typedMsgCurrentPos) <= len) {
                        tmp_size = base_message_rx.size - typedMsgCurrentPos;
                      } else {
                        tmp_size = len;
                      }

                      if (received_header == true) {
                        switch (codec) {
                          case OPUS:
                          case FLAC: {
                            memcpy(&decoderChunk.inData[payloadOffset], start, tmp_size);
                            payloadOffset += tmp_size;
                            decoderChunk.outData = NULL;
                            decoderChunk.type = SNAPCAST_MESSAGE_WIRE_CHUNK;

                            break;
                          }

                          case PCM: {
                            size_t _tmp = tmp_size;

                            offset = 0;

                            if (pcmData == NULL) {
                              if (allocate_pcm_chunk_memory(&pcmData, wire_chnk.size) < 0) {
                                ESP_LOGW(TAG, "DECODE_DROP: PCM alloc failed, size=%d, chunk lost",
                                         wire_chnk.size);
                                s_dec_drop++;
                                pcmData = NULL;
                              }

                              tmpData = 0;
                              payloadDataShift = 3;
                              payloadOffset = 0;
                            }

                            while (_tmp--) {
                              tmpData |= ((uint32_t) start[offset++] << (8 * payloadDataShift));

                              payloadDataShift--;
                              if (payloadDataShift < 0) {
                                payloadDataShift = 3;

                                if ((pcmData) && (pcmData->fragment->payload)) {
                                  volatile uint32_t *sample;
                                  uint8_t dummy1;
                                  uint32_t dummy2 = 0;

                                  dummy1 = tmpData >> 24;
                                  dummy2 |= (uint32_t) dummy1 << 16;
                                  dummy1 = tmpData >> 16;
                                  dummy2 |= (uint32_t) dummy1 << 24;
                                  dummy1 = tmpData >> 8;
                                  dummy2 |= (uint32_t) dummy1 << 0;
                                  dummy1 = tmpData >> 0;
                                  dummy2 |= (uint32_t) dummy1 << 8;
                                  tmpData = dummy2;

                                  sample = (volatile uint32_t *) (&(pcmData->fragment->payload[payloadOffset]));
                                  *sample = (volatile uint32_t) tmpData;

                                  payloadOffset += 4;
                                }

                                tmpData = 0;
                              }
                            }

                            break;
                          }

                          default: {
                            ESP_LOGE(TAG, "Decoder (1) not supported");

                            return;

                            break;
                          }
                        }
                      }

                      typedMsgCurrentPos += tmp_size;
                      start += tmp_size;

                      len -= tmp_size;

                      if (typedMsgCurrentPos >= base_message_rx.size) {
                        if (received_header == true) {
                          switch (codec) {
                            case OPUS: {
                              int frame_size = -1;
                              int samples_per_frame;
                              opus_int16 *audio = NULL;

                              samples_per_frame = opus_packet_get_samples_per_frame(decoderChunk.inData, scSet.sr);
                              if (samples_per_frame < 0) {
                                ESP_LOGE(TAG, "couldn't get samples per frame count "
                                              "of packet");
                              }

                              scSet.chkInFrames = samples_per_frame;

                              size_t bytes;
                              int decode_retries = 0;
                              bool decode_ok = false;
                              do {
                                bytes = samples_per_frame * (scSet.ch * scSet.bits >> 3);

                                int alloc_retries = 0;
                                while ((audio = (opus_int16 *) realloc(audio, bytes)) == NULL) {
                                  if (++alloc_retries > 50) {
                                    ESP_LOGW(TAG, "DECODE_DROP: OPUS realloc(%d) failed after %d retries",
                                             (int)bytes, alloc_retries);
                                    break;
                                  }
                                  vTaskDelay(pdMS_TO_TICKS(1));
                                }
                                if (!audio) break;

                                frame_size = opus_decode(opusDecoder, decoderChunk.inData, decoderChunk.bytes,
                                                         (opus_int16 *) audio, samples_per_frame, 0);

                                if (frame_size >= 0) {
                                  decode_ok = true;
                                } else if (++decode_retries > 3) {
                                  ESP_LOGW(TAG, "DECODE_DROP: OPUS decode failed %d times, err=%d, dropping chunk",
                                           decode_retries, frame_size);
                                  s_dec_drop++;
                                  break;
                                }
                                samples_per_frame <<= 1;
                              } while (!decode_ok);

                              free(decoderChunk.inData);
                              decoderChunk.inData = NULL;

                              if (!decode_ok) {
                                if (audio) { free(audio); audio = NULL; }
                                break;
                              }

                              pcm_chunk_message_t *new_pcmChunk = NULL;

                              if (allocate_pcm_chunk_memory(&new_pcmChunk, bytes) < 0) {
                                ESP_LOGW(TAG, "DECODE_DROP: OPUS alloc failed, size=%d, chunk lost",
                                         (int)bytes);
                                s_dec_drop++;
                                pcmData = NULL;
                              } else {
                                new_pcmChunk->timestamp = wire_chnk.timestamp;

                                if (new_pcmChunk->fragment->payload) {
                                  volatile uint32_t *sample;
                                  uint32_t tmpData;
                                  uint32_t cnt = 0;

                                  for (int i = 0; i < bytes; i += 4) {
                                    sample = (volatile uint32_t *) (&(new_pcmChunk->fragment->payload[i]));
                                    tmpData = (((uint32_t) audio[cnt] << 16) & 0xFFFF0000) |
                                              (((uint32_t) audio[cnt + 1] << 0) & 0x0000FFFF);
                                    *sample = (volatile uint32_t) tmpData;

                                    cnt += 2;
                                  }
                                }

                                free(audio);
                                audio = NULL;

#if CONFIG_USE_DSP_PROCESSOR
                                if (new_pcmChunk->fragment->payload) {
                                  dsp_processor_worker(new_pcmChunk->fragment->payload, new_pcmChunk->fragment->size,
                                                       scSet.sr);
                                }
#endif

                                insert_pcm_chunk(new_pcmChunk);
                                s_dec_ok++;
                              }

                              if (player_send_snapcast_setting(&scSet) != pdPASS) {
                                ESP_LOGE(TAG, "Failed to notify "
                                              "sync task about "
                                              "codec. Did you "
                                              "init player?");

                                return;
                              }

                              break;
                            }

                            case FLAC: {
                              isCachedChunk = true;
                              cachedBlocks = 0;
                              int flac_errors = 0;

                              while (decoderChunk.bytes > 0) {
                                int prev_bytes = decoderChunk.bytes;
                                if (FLAC__stream_decoder_process_single(flacDecoder) == 0) {
                                  flac_errors++;
                                  ESP_LOGE(TAG,
                                           "DECODE_ERR: FLAC process_single failed, bytes_left=%d attempt=%d",
                                           decoderChunk.bytes, flac_errors);
                                  s_dec_drop++;

                                  if (flac_errors >= 3) {
                                    ESP_LOGW(TAG, "DECODE_DROP: FLAC failed %d times, dropping chunk", flac_errors);
                                    break;
                                  }
                                  vTaskDelay(pdMS_TO_TICKS(10));
                                } else if (decoderChunk.bytes == prev_bytes) {

                                  flac_errors++;
                                  if (flac_errors >= 3) {
                                    ESP_LOGW(TAG, "DECODE_DROP: FLAC stalled (no progress), dropping chunk");
                                    s_dec_drop++;
                                    break;
                                  }
                                } else {
                                  flac_errors = 0;
                                }
                              }

                              if (decoderChunk.bytes > 0) {

                                if (decoderChunk.inData) { free(decoderChunk.inData); decoderChunk.inData = NULL; }
                                decoderChunk.bytes = 0;
                                if (pcmChunk.outData) { free(pcmChunk.outData); pcmChunk.outData = NULL; }
                                pcmChunk.bytes = 0;
                                break;
                              }

                              if ((cachedBlocks > 0) && (scSet.sr != 0)) {
                                uint64_t diffUs = 1000000ULL * cachedBlocks / scSet.sr;

                                uint64_t timestamp = 1000000ULL * wire_chnk.timestamp.sec + wire_chnk.timestamp.usec;

                                timestamp = timestamp - diffUs;

                                wire_chnk.timestamp.sec = timestamp / 1000000ULL;
                                wire_chnk.timestamp.usec = timestamp % 1000000ULL;
                              }

                              pcm_chunk_message_t *new_pcmChunk;
                              int32_t ret = allocate_pcm_chunk_memory(&new_pcmChunk, pcmChunk.bytes);

                              scSet.chkInFrames = FLAC__stream_decoder_get_blocksize(flacDecoder);

                              if (ret == 0) {
                                pcm_chunk_fragment_t *fragment = new_pcmChunk->fragment;
                                uint32_t fragmentCnt = 0;

                                if (fragment->payload != NULL) {
                                  uint32_t frames = pcmChunk.bytes / (scSet.ch * (scSet.bits / 8));

                                  for (int i = 0; i < frames; i++) {

                                    uint32_t tmpData;
                                    memcpy(&tmpData, &pcmChunk.outData[fragmentCnt], (scSet.ch * (scSet.bits / 8)));

                                    if (fragment != NULL) {
                                      volatile uint32_t *test =
                                          (volatile uint32_t *) (&(fragment->payload[fragmentCnt]));
                                      *test = (volatile uint32_t) tmpData;
                                    }

                                    fragmentCnt += (scSet.ch * (scSet.bits / 8));
                                    if (fragmentCnt >= fragment->size) {
                                      fragmentCnt = 0;

                                      fragment = fragment->nextFragment;
                                    }
                                  }
                                }

                                new_pcmChunk->timestamp = wire_chnk.timestamp;

#if CONFIG_USE_DSP_PROCESSOR
                                if (new_pcmChunk->fragment->payload) {
                                  dsp_processor_worker(new_pcmChunk->fragment->payload, new_pcmChunk->fragment->size,
                                                       scSet.sr);
                                }

#endif

                                insert_pcm_chunk(new_pcmChunk);
                                s_dec_ok++;
                              }

                              free(pcmChunk.outData);
                              pcmChunk.outData = NULL;
                              pcmChunk.bytes = 0;

                              if (player_send_snapcast_setting(&scSet) != pdPASS) {
                                ESP_LOGE(TAG, "Failed to "
                                              "notify "
                                              "sync task "
                                              "about "
                                              "codec. Did you "
                                              "init player?");

                                return;
                              }

                              break;
                            }

                            case PCM: {
                              size_t decodedSize = wire_chnk.size;

                              if (pcmData) {
                                pcmData->timestamp = wire_chnk.timestamp;
                              }

                              scSet.chkInFrames = decodedSize / ((size_t) scSet.ch * (size_t) (scSet.bits / 8));

                              if (player_send_snapcast_setting(&scSet) != pdPASS) {
                                ESP_LOGE(TAG, "Failed to notify "
                                              "sync task about "
                                              "codec. Did you "
                                              "init player?");

                                return;
                              }

#if CONFIG_USE_DSP_PROCESSOR
                              if ((pcmData) && (pcmData->fragment->payload)) {
                                dsp_processor_worker(pcmData->fragment->payload, pcmData->fragment->size, scSet.sr);
                              }
#endif

                              if (pcmData) {
                                insert_pcm_chunk(pcmData);
                                s_dec_ok++;
                              }

                              pcmData = NULL;

                              free(decoderChunk.inData);
                              decoderChunk.inData = NULL;

                              break;
                            }

                            default: {
                              ESP_LOGE(TAG, "Decoder (2) not "
                                            "supported");

                              return;

                              break;
                            }
                          }
                        }

                        state = BASE_MESSAGE_STATE;
                        internalState = 0;

                        typedMsgCurrentPos = 0;
                      }

                      break;
                    }

                    default: {
                      ESP_LOGE(TAG, "wire chunk decoder "
                                    "shouldn't get here");

                      break;
                    }
                  }

                  break;
                }

                case SNAPCAST_MESSAGE_CODEC_HEADER: {
                  switch (internalState) {
                    case 0: {
                      typedMsgLen = *start & 0xFF;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 1: {
                      typedMsgLen |= (*start & 0xFF) << 8;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 2: {
                      typedMsgLen |= (*start & 0xFF) << 16;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 3: {
                      typedMsgLen |= (*start & 0xFF) << 24;

                      codecString = (char *) malloc(typedMsgLen + 1);

                      if (codecString == NULL) {
                        ESP_LOGE(TAG, "couldn't get memory "
                                      "for codec string");

                        return;
                      }

                      offset = 0;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 4: {
                      if (len >= typedMsgLen) {
                        memcpy(&codecString[offset], start, typedMsgLen);

                        offset += typedMsgLen;

                        typedMsgCurrentPos += typedMsgLen;
                        start += typedMsgLen;

                        len -= typedMsgLen;
                      } else {
                        memcpy(&codecString[offset], start, typedMsgLen);

                        offset += len;

                        typedMsgCurrentPos += len;
                        start += len;

                        len -= len;
                      }

                      if (offset == typedMsgLen) {

                        codecString[typedMsgLen] = 0;

                        if (strcmp(codecString, "opus") == 0) {
                          codec = OPUS;
                        } else if (strcmp(codecString, "flac") == 0) {
                          codec = FLAC;
                        } else if (strcmp(codecString, "pcm") == 0) {
                          codec = PCM;
                        } else {
                          codec = NONE;

                          ESP_LOGI(TAG, "Codec : %s not supported", codecString);
                          ESP_LOGI(TAG, "Change encoder codec to "
                                        "opus, flac or pcm in "
                                        "/etc/snapserver.conf on "
                                        "server");

                          return;
                        }

                        free(codecString);
                        codecString = NULL;

                        internalState++;
                      }

                      if (len == 0) {
                        break;
                      }
                    }

                    case 5: {
                      typedMsgLen = *start & 0xFF;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 6: {
                      typedMsgLen |= (*start & 0xFF) << 8;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 7: {
                      typedMsgLen |= (*start & 0xFF) << 16;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 8: {
                      typedMsgLen |= (*start & 0xFF) << 24;

                      codecPayload = (char *) malloc(typedMsgLen);

                      if (codecPayload == NULL) {
                        ESP_LOGE(TAG, "couldn't get memory "
                                      "for codec payload");

                        return;
                      }

                      offset = 0;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 9: {
                      if (len >= typedMsgLen) {
                        memcpy(&codecPayload[offset], start, typedMsgLen);

                        offset += typedMsgLen;

                        typedMsgCurrentPos += typedMsgLen;
                        start += typedMsgLen;

                        len -= typedMsgLen;
                      } else {
                        memcpy(&codecPayload[offset], start, len);

                        offset += len;

                        typedMsgCurrentPos += len;
                        start += len;

                        len -= len;
                      }

                      if (offset == typedMsgLen) {

                        if (flacDecoder != NULL) {
                          FLAC__stream_decoder_finish(flacDecoder);
                          FLAC__stream_decoder_delete(flacDecoder);
                          flacDecoder = NULL;
                        }

                        if (opusDecoder != NULL) {
                          opus_decoder_destroy(opusDecoder);
                          opusDecoder = NULL;
                        }

                        if (codec == OPUS) {
                          uint16_t channels;
                          uint32_t rate;
                          uint16_t bits;

                          memcpy(&rate, codecPayload + 4, sizeof(rate));
                          memcpy(&bits, codecPayload + 8, sizeof(bits));
                          memcpy(&channels, codecPayload + 10, sizeof(channels));

                          scSet.codec = codec;
                          scSet.bits = (i2s_data_bit_width_t) bits;
                          scSet.ch = channels;
                          scSet.sr = rate;

                          ESP_LOGI(TAG, "Opus sample format: %ld:%d:%d\n", rate, bits, channels);

                          int error = 0;

                          opusDecoder = opus_decoder_create(scSet.sr, scSet.ch, &error);
                          if (error != 0) {
                            ESP_LOGI(TAG, "Failed to init opus coder");
                            return;
                          }

                          ESP_LOGI(TAG, "Initialized opus Decoder: %d", error);
                        } else if (codec == FLAC) {
                          decoderChunk.bytes = typedMsgLen;
                          decoderChunk.inData = (uint8_t *) malloc(decoderChunk.bytes);
                          memcpy(decoderChunk.inData, codecPayload, typedMsgLen);
                          decoderChunk.outData = NULL;
                          decoderChunk.type = SNAPCAST_MESSAGE_CODEC_HEADER;

                          flacDecoder = FLAC__stream_decoder_new();
                          if (flacDecoder == NULL) {
                            ESP_LOGE(TAG, "Failed to init flac decoder");
                            return;
                          }

                          FLAC__StreamDecoderInitStatus init_status = FLAC__stream_decoder_init_stream(
                              flacDecoder, read_callback, NULL, NULL, NULL, NULL, write_callback, metadata_callback,
                              error_callback, &scSet);
                          if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
                            ESP_LOGE(TAG, "ERROR: initializing decoder: %s\n",
                                     FLAC__StreamDecoderInitStatusString[init_status]);

                            return;
                          }

                          FLAC__stream_decoder_process_until_end_of_metadata(flacDecoder);

                        } else if (codec == PCM) {
                          uint16_t channels;
                          uint32_t rate;
                          uint16_t bits;

                          memcpy(&channels, codecPayload + 22, sizeof(channels));
                          memcpy(&rate, codecPayload + 24, sizeof(rate));
                          memcpy(&bits, codecPayload + 34, sizeof(bits));

                          scSet.codec = codec;
                          scSet.bits = (i2s_data_bit_width_t) bits;
                          scSet.ch = channels;
                          scSet.sr = rate;

                          ESP_LOGI(TAG, "pcm sampleformat: %ld:%d:%d", scSet.sr, scSet.bits, scSet.ch);
                        } else {
                          ESP_LOGE(TAG, "codec header decoder "
                                        "shouldn't get here after "
                                        "codec string was detected");

                          return;
                        }

                        free(codecPayload);
                        codecPayload = NULL;

                        if (snap_component && snap_component->source_) {
                          snap_component->source_->set_audio_stream_info(
                              audio::AudioStreamInfo(scSet.bits, scSet.ch, scSet.sr));
                          ESP_LOGI(TAG, "set_audio_stream_info: %ld Hz %d-bit %d-ch",
                                   scSet.sr, scSet.bits, scSet.ch);
#ifdef USE_SHARED_AUDIO_EQ
                          eq_set_sample_rate(scSet.sr);
#endif
                        }

                        if (player_send_snapcast_setting(&scSet) != pdPASS) {
                          ESP_LOGE(TAG, "Failed to notify sync task. "
                                        "Did you init player?");

                          return;
                        }

                        state = BASE_MESSAGE_STATE;
                        internalState = 0;

                        received_header = true;
                      }

                      break;
                    }

                    default: {
                      ESP_LOGE(TAG, "codec header decoder "
                                    "shouldn't get here");

                      break;
                    }
                  }

                  break;
                }

                case SNAPCAST_MESSAGE_SERVER_SETTINGS: {
                  switch (internalState) {
                    case 0: {
                      typedMsgLen = *start & 0xFF;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 1: {
                      typedMsgLen |= (*start & 0xFF) << 8;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 2: {
                      typedMsgLen |= (*start & 0xFF) << 16;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 3: {
                      typedMsgLen |= (*start & 0xFF) << 24;

                      serverSettingsString = (char *) malloc(typedMsgLen + 1);
                      if (serverSettingsString == NULL) {
                        ESP_LOGE(TAG, "couldn't get memory for "
                                      "server settings string");
                      }

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      offset = 0;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 4: {
                      size_t tmpSize = base_message_rx.size - typedMsgCurrentPos;

                      if (len > 0) {
                        if (tmpSize < len) {
                          if (serverSettingsString) {
                            memcpy(&serverSettingsString[offset], start, tmpSize);
                          }
                          offset += tmpSize;

                          start += tmpSize;

                          typedMsgCurrentPos += tmpSize;
                          len -= tmpSize;
                        } else {
                          if (serverSettingsString) {
                            memcpy(&serverSettingsString[offset], start, len);
                          }
                          offset += len;

                          start += len;

                          typedMsgCurrentPos += len;
                          len = 0;
                        }
                      }

                      if (typedMsgCurrentPos >= base_message_rx.size) {
                        if (serverSettingsString) {

                          serverSettingsString[typedMsgLen] = 0;

                          result = server_settings_message_deserialize(&server_settings_message, serverSettingsString);
                          if (result) {
                            ESP_LOGE(TAG,
                                     "Failed to read server "
                                     "settings: %d",
                                     result);
                          } else {

                            ESP_LOGI(TAG, "Buffer length:  %ld", server_settings_message.buffer_ms);
                            ESP_LOGI(TAG, "Latency:        %ld", server_settings_message.latency);
                            ESP_LOGI(TAG, "Mute:           %d", server_settings_message.muted);
                            ESP_LOGI(TAG, "Setting volume: %ld", server_settings_message.volume);
                          }

                          if (scSet.muted != server_settings_message.muted) {
#if SNAPCAST_USE_SOFT_VOL
                            if (server_settings_message.muted) {
                              dsp_processor_set_volome(0.0);
                            } else {
                              dsp_processor_set_volome((double) server_settings_message.volume / 100);
                            }
#endif

                          }

                          if (scSet.volume != server_settings_message.volume) {
#if SNAPCAST_USE_SOFT_VOL
                            if (!server_settings_message.muted) {
                              dsp_processor_set_volome((double) server_settings_message.volume / 100);
                            }
#else
                            audio_set_volume(server_settings_message.volume);
#endif
                          }

                          scSet.cDacLat_ms = server_settings_message.latency;
                          scSet.buf_ms = server_settings_message.buffer_ms;
                          scSet.muted = server_settings_message.muted;
                          scSet.volume = server_settings_message.volume;

                          if (player_send_snapcast_setting(&scSet) != pdPASS) {
                            ESP_LOGE(TAG, "Failed to notify sync task. "
                                          "Did you init player?");

                            return;
                          }

                          free(serverSettingsString);
                          serverSettingsString = NULL;
                        }

                        state = BASE_MESSAGE_STATE;
                        internalState = 0;

                        typedMsgCurrentPos = 0;
                      }

                      break;
                    }

                    default: {
                      ESP_LOGE(TAG, "server settings decoder "
                                    "shouldn't get here");

                      break;
                    }
                  }

                  break;
                }

                case SNAPCAST_MESSAGE_STREAM_TAGS: {
                  size_t tmpSize = base_message_rx.size - typedMsgCurrentPos;

                  if (tmpSize < len) {
                    start += tmpSize;

                    typedMsgCurrentPos += tmpSize;
                    len -= tmpSize;
                  } else {
                    start += len;

                    typedMsgCurrentPos += len;
                    len = 0;
                  }

                  if (typedMsgCurrentPos >= base_message_rx.size) {

                    typedMsgCurrentPos = 0;

                    state = BASE_MESSAGE_STATE;
                    internalState = 0;
                  }

                  break;
                }

                case SNAPCAST_MESSAGE_TIME: {
                  switch (internalState) {
                    case 0: {
                      time_message_rx.latency.sec = *start & 0xFF;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 1: {
                      time_message_rx.latency.sec |= (*start & 0xFF) << 8;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 2: {
                      time_message_rx.latency.sec |= (*start & 0xFF) << 16;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 3: {
                      time_message_rx.latency.sec |= (*start & 0xFF) << 24;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 4: {
                      time_message_rx.latency.usec = *start & 0xFF;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 5: {
                      time_message_rx.latency.usec |= (*start & 0xFF) << 8;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 6: {
                      time_message_rx.latency.usec |= (*start & 0xFF) << 16;

                      typedMsgCurrentPos++;
                      start++;

                      len--;

                      internalState++;

                      if (len == 0) {
                        break;
                      }
                    }

                    case 7: {
                      time_message_rx.latency.usec |= (*start & 0xFF) << 24;

                      typedMsgCurrentPos++;
                      start++;

                      len--;
                      if (typedMsgCurrentPos >= base_message_rx.size) {

                        typedMsgCurrentPos = 0;

                        state = BASE_MESSAGE_STATE;
                        internalState = 0;

                        trx = (int64_t) base_message_rx.received.sec * 1000000LL +
                              (int64_t) base_message_rx.received.usec;
                        ttx = (int64_t) base_message_rx.sent.sec * 1000000LL + (int64_t) base_message_rx.sent.usec;
                        tdif = trx - ttx;
                        ttx =
                            (int64_t) time_message_rx.latency.sec * 1000000LL + (int64_t) time_message_rx.latency.usec;
                        tmpDiffToServer = (ttx - tdif) / 2;

                        int64_t diff;

                        diff = now - lastTimeSync;
                        if (diff > 60000000LL) {
                          ESP_LOGW(TAG, "Last time sync older "
                                        "than a minute. "
                                        "Clearing time buffer");

                          reset_latency_buffer();

                          timeout = FAST_SYNC_LATENCY_BUF;
                          netconn_set_recvtimeout(lwipNetconn, timeout / 1000);
                        }

#if USE_TIMEFILTER
                        player_latency_insert(tmpDiffToServer, (tdif + ttx) / 2, trx);
#else
                        player_latency_insert(tmpDiffToServer);
#endif

                        lastTimeSync = now;

                        if (received_header == true) {
                          bool is_full = false;
                          latency_buffer_full(&is_full);
                          if ((is_full == true) && (timeout < NORMAL_SYNC_LATENCY_BUF)) {
                            timeout = NORMAL_SYNC_LATENCY_BUF;
                            netconn_set_recvtimeout(lwipNetconn, timeout / 1000);

                            ESP_LOGI(TAG, "latency buffer full");
                          } else if ((is_full == false) && (timeout > FAST_SYNC_LATENCY_BUF)) {
                            timeout = FAST_SYNC_LATENCY_BUF;
                            netconn_set_recvtimeout(lwipNetconn, timeout / 1000);

                            ESP_LOGI(TAG, "latency buffer not full");
                          }
                        }
                      } else {
                        ESP_LOGE(TAG,
                                 "error time message, this "
                                 "shouldn't happen! %d %ld",
                                 typedMsgCurrentPos, base_message_rx.size);

                        typedMsgCurrentPos = 0;

                        state = BASE_MESSAGE_STATE;
                        internalState = 0;
                      }

                      break;
                    }

                    default: {
                      ESP_LOGE(TAG,
                               "time message decoder shouldn't "
                               "get here %d %ld %ld",
                               typedMsgCurrentPos, base_message_rx.size, internalState);

                      break;
                    }
                  }

                  break;
                }

                default: {
                  typedMsgCurrentPos++;
                  start++;

                  len--;

                  if (typedMsgCurrentPos >= base_message_rx.size) {
                    ESP_LOGI(TAG, "done unknown typed message %d", base_message_rx.type);

                    state = BASE_MESSAGE_STATE;
                    internalState = 0;

                    typedMsgCurrentPos = 0;
                  }

                  break;
                }
              }

              break;
            }

            default: {
              break;
            }
          }

          if (rc1 != ERR_OK) {
            break;
          }
        }
      } while (netbuf_next(firstNetBuf) >= 0);

      netbuf_delete(firstNetBuf);

      {
        int64_t loop_us = esp_timer_get_time() - recv_start_us;
        if (loop_us > s_max_loop_us) s_max_loop_us = loop_us;
      }

      if (rc1 != ERR_OK) {
        ESP_LOGE(TAG, "Data error, closing netconn");

        netconn_close(lwipNetconn);

        break;
      }
    }
  }

  ESP_LOGI(TAG, "http_get_task exiting (stop requested)");
  if (lwipNetconn) {
    netconn_close(lwipNetconn);
    netconn_delete(lwipNetconn);
    lwipNetconn = NULL;
  }
  snap_component->http_get_task_handle_ = nullptr;
  vTaskDelete(NULL);
}

}
}

#endif
#endif
