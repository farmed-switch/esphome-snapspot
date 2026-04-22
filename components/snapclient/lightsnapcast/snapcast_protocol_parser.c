#include "snapcast_protocol_parser.h"

#include "esp_log.h"
#include "freertos/task.h"
#include <string.h>

static inline bool read_byte(snapcast_protocol_parser_t *parser, char *dest) {
    char byte;
    if (parser->get_byte_function(parser->get_byte_context, &byte) != 0) {
        return false;
    }
    *dest = byte;
    return true;
}

static inline bool read_uint16_le(snapcast_protocol_parser_t *parser, uint16_t *dest) {
    char bytes[2];
    for (int i = 0; i < 2; i++) {
        if (parser->get_byte_function(parser->get_byte_context, &bytes[i]) != 0) {
            return false;
        }
    }
    *dest = (uint16_t)((bytes[0] & 0xFF) | ((bytes[1] & 0xFF) << 8));
    return true;
}

static inline bool read_uint32_le(snapcast_protocol_parser_t *parser, uint32_t *dest) {
    char bytes[4];
    for (int i = 0; i < 4; i++) {
        if (parser->get_byte_function(parser->get_byte_context, &bytes[i]) != 0) {
            return false;
        }
    }
    *dest = (uint32_t)(  (bytes[0] & 0xFF)
                       | ((bytes[1] & 0xFF) <<  8)
                       | ((bytes[2] & 0xFF) << 16)
                       | ((bytes[3] & 0xFF) << 24));
    return true;
}

static inline bool read_timestamp(snapcast_protocol_parser_t *parser, tv_t *ts) {
    return read_uint32_le(parser, (uint32_t *)&ts->sec) &&
           read_uint32_le(parser, (uint32_t *)&ts->usec);
}

static inline bool read_data(snapcast_protocol_parser_t *parser, uint8_t *dest, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        if (parser->get_byte_function(parser->get_byte_context, (char *)&dest[i]) != 0) {
            return false;
        }
    }
    return true;
}

static const char* TAG = "SNAPCAST_PROTOCOL_PARSER";

parser_return_state_t parse_base_message(snapcast_protocol_parser_t* parser,
                                         base_message_t* base_message_rx) {
  if (!read_uint16_le(parser, &base_message_rx->type)) return PARSER_RESTART_CONNECTION;
  if (!read_uint16_le(parser, &base_message_rx->id)) return PARSER_RESTART_CONNECTION;
  if (!read_uint16_le(parser, &base_message_rx->refersTo)) return PARSER_RESTART_CONNECTION;
  if (!read_timestamp(parser, &base_message_rx->sent)) return PARSER_RESTART_CONNECTION;
  if (!read_timestamp(parser, &base_message_rx->received)) return PARSER_RESTART_CONNECTION;
  if (!read_uint32_le(parser, &base_message_rx->size)) return PARSER_RESTART_CONNECTION;

  return PARSER_OK;
}

parser_return_state_t parse_wire_chunk_message(snapcast_protocol_parser_t* parser,
                                               base_message_t* base_message_rx,
                                               codec_type_t codec,
                                               pcm_chunk_message_t** pcmData,
                                               wire_chunk_message_t* wire_chnk,
                                               decoderData_t* decoderChunk) {

  if (!read_timestamp(parser, &wire_chnk->timestamp)) return PARSER_RESTART_CONNECTION;
  if (!read_uint32_le(parser, (uint32_t *)&wire_chnk->size)) return PARSER_RESTART_CONNECTION;

  decoderChunk->bytes = wire_chnk->size;

  if (decoderChunk->bytes > 131072) {
    ESP_LOGW(TAG, "PARSE_DROP: chunk size %d > 128KB, corrupt? dropping",
             (int)decoderChunk->bytes);
    decoderChunk->bytes = 0;
    decoderChunk->inData = NULL;
    return PARSER_RESTART_CONNECTION;
  }

  {
    int alloc_retries = 0;
    while (!decoderChunk->inData) {
      decoderChunk->inData = (uint8_t*)malloc(decoderChunk->bytes);
      if (!decoderChunk->inData) {
        if (++alloc_retries > 50) {
          ESP_LOGW(TAG, "PARSE_DROP: malloc(%d) failed after %d retries, dropping chunk",
                   (int)decoderChunk->bytes, alloc_retries);
          decoderChunk->bytes = 0;
          return PARSER_RESTART_CONNECTION;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
      }
    }
  }

  uint32_t payloadOffset = 0;
  uint32_t tmpData = 0;
  int32_t payloadDataShift = 0;
  size_t tmp_size = base_message_rx->size - 12;

  switch (codec) {
    case OPUS:
    case FLAC: {
      if (!read_data(parser, decoderChunk->inData, tmp_size)) return PARSER_RESTART_CONNECTION;
      payloadOffset += tmp_size;
      decoderChunk->outData = NULL;
      decoderChunk->type = SNAPCAST_MESSAGE_WIRE_CHUNK;

      break;
    }

    case PCM: {
      size_t _tmp = tmp_size;

      if (*pcmData == NULL) {
        if (allocate_pcm_chunk_memory(pcmData, wire_chnk->size) < 0) {
          *pcmData = NULL;
        }

        tmpData = 0;
        payloadDataShift = 3;
        payloadOffset = 0;
      }

      while (_tmp--) {
        char tmp_val;
        if (!read_byte(parser, &tmp_val)) return PARSER_RESTART_CONNECTION;

        tmpData |= ((uint32_t)tmp_val << (8 * payloadDataShift));

        payloadDataShift--;
        if (payloadDataShift < 0) {
          payloadDataShift = 3;

          if ((*pcmData) && ((*pcmData)->fragment->payload)) {
            volatile uint32_t* sample;
            uint8_t dummy1;
            uint32_t dummy2 = 0;

            dummy1 = tmpData >> 24;
            dummy2 |= (uint32_t)dummy1 << 16;
            dummy1 = tmpData >> 16;
            dummy2 |= (uint32_t)dummy1 << 24;
            dummy1 = tmpData >> 8;
            dummy2 |= (uint32_t)dummy1 << 0;
            dummy1 = tmpData >> 0;
            dummy2 |= (uint32_t)dummy1 << 8;
            tmpData = dummy2;

            sample = (volatile uint32_t *)(&((*pcmData)->fragment->payload[payloadOffset]));
            *sample = (volatile uint32_t)tmpData;

            payloadOffset += 4;
          }

          tmpData = 0;
        }
      }

      break;
    }
    default: {
      ESP_LOGE(TAG, "Decoder (1) not supported. This should never happen!");

      esp_restart();
    }
  }

  return PARSER_OK;
}

parser_return_state_t parse_codec_header_message(
    snapcast_protocol_parser_t* parser,
    bool* received_codec_header, codec_type_t* codec,
    char** codecPayload, uint32_t* codecPayloadLen) {
  *received_codec_header = false;

  uint32_t codecStringLen = 0;
  if (!read_uint32_le(parser, &codecStringLen)) return PARSER_RESTART_CONNECTION;

  char codecString[8];

  if (codecStringLen + 1 > sizeof(codecString)) {
    if (!read_data(parser, (uint8_t *)codecString, sizeof(codecString)-1)) return PARSER_RESTART_CONNECTION;

    codecString[sizeof(codecString)-1] = 0;
    ESP_LOGE(TAG, "Codec : %s... not supported", codecString);
    ESP_LOGI(TAG,
             "Change encoder codec to "
             "opus, flac or pcm in "
             "/etc/snapserver.conf on "
             "server");

    return PARSER_RESTART_CONNECTION;
  }
  if (!read_data(parser, (uint8_t *)codecString, codecStringLen)) return PARSER_RESTART_CONNECTION;

  codecString[codecStringLen] = 0;

  if (strcmp(codecString, "opus") == 0) {
    *codec = OPUS;
  } else if (strcmp(codecString, "flac") == 0) {
    *codec = FLAC;
  } else if (strcmp(codecString, "pcm") == 0) {
    *codec = PCM;
  } else {
    *codec = NONE;

    ESP_LOGI(TAG, "Codec : %s not supported", codecString);
    ESP_LOGI(TAG,
             "Change encoder codec to "
             "opus, flac or pcm in "
             "/etc/snapserver.conf on "
             "server");

    return PARSER_RESTART_CONNECTION;
  }

  if (!read_uint32_le(parser, codecPayloadLen)) return PARSER_RESTART_CONNECTION;

  *codecPayload = malloc(*codecPayloadLen);

  if (*codecPayload == NULL) {
    ESP_LOGE(TAG,
             "couldn't get memory "
             "for codec payload");

    esp_restart();
  }

  if (!read_data(parser, (uint8_t *)*codecPayload, *codecPayloadLen)) return PARSER_RESTART_CONNECTION;

  *received_codec_header = true;

  return PARSER_OK;
}

parser_return_state_t parse_sever_settings_message(
    snapcast_protocol_parser_t* parser, base_message_t* base_message_rx,
    server_settings_message_t* server_settings_message) {
  uint32_t typedMsgLen;
  char* serverSettingsString = NULL;
  if (!read_uint32_le(parser, &typedMsgLen)) return PARSER_RESTART_CONNECTION;

  serverSettingsString = malloc(typedMsgLen + 1);
  if (serverSettingsString == NULL) {
    ESP_LOGE(TAG,
             "couldn't get memory for "
             "server settings string");

    esp_restart();
  }

  size_t tmpSize = base_message_rx->size - 4;

  if (!read_data(parser, (uint8_t *)serverSettingsString, tmpSize)) {
    free(serverSettingsString);

    return PARSER_RESTART_CONNECTION;
  }

  serverSettingsString[typedMsgLen] = 0;

  int deserialization_result;

  deserialization_result = server_settings_message_deserialize(
      server_settings_message, serverSettingsString);

  free(serverSettingsString);

  if (deserialization_result) {
    ESP_LOGE(TAG,
             "Failed to read server "
             "settings: %d",
             deserialization_result);

    esp_restart();
  }

  return PARSER_OK;

}

parser_return_state_t parse_time_message(snapcast_protocol_parser_t* parser,
                                         base_message_t* base_message_rx,
                                         time_message_t* time_message_rx) {
  if (!read_timestamp(parser, &time_message_rx->latency)) return PARSER_RESTART_CONNECTION;

  if (base_message_rx->size < 8) {
    ESP_LOGE(TAG,
             "error time message, this shouldn't happen! %d %ld",
             8, base_message_rx->size);
    return PARSER_RESTART_CONNECTION;
  }

  return PARSER_OK;
}

parser_return_state_t parser_skip_typed_message(snapcast_protocol_parser_t* parser,
                                            base_message_t* base_message_rx) {

  char dummy_byte;
  for (uint32_t i = 0; i < base_message_rx->size; i++) {
    if (!read_byte(parser, &dummy_byte)) return PARSER_RESTART_CONNECTION;
  }

  ESP_LOGI(TAG, "done skipping typed message %d", base_message_rx->type);

  return PARSER_OK;
}
