#pragma once
#include <cstdint>
#include <cstddef>
#include "driver/i2s_std.h"
#include "esp_log.h"

namespace esphome::spotify_connect {

class ESPHomeAudioSink {
 public:
  void set_i2s_handle(i2s_chan_handle_t handle) { tx_handle_ = handle; }

  size_t feed_pcm(uint8_t *data, size_t bytes) {
    if (!tx_handle_) return 0;
    size_t written = 0;
    esp_err_t err = i2s_channel_write(tx_handle_, data, bytes, &written, pdMS_TO_TICKS(100));
    if (err == ESP_ERR_INVALID_STATE) {

      return bytes;
    }
    return written;
  }

 private:
  i2s_chan_handle_t tx_handle_{nullptr};
};

}
