/**
 * AudioOutput Adapter for ESPHome snapspot
 * Wraps i2s_chan_handle_t with the same interface as snapspot-esp32's AudioOutput,
 * so snapcast_client.cpp compiles without modification.
 *
 * Ported from snapspot-esp32/main/audio_output.h (interface only — new implementation)
 */

#pragma once

#include "esp_err.h"
#include "driver/i2s_std.h"
#include "driver/i2s_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <cstdint>

// Forward declaration so initialize() signature compiles without pulling in config.h
namespace config { struct Settings; }

namespace esphome {
namespace snapspot {

// Forward declaration (mirrors snapspot-esp32's forward decl pattern)
namespace sources {
enum class SourceId : uint8_t;
}

// DAC control message — pushed to audio_q_hdl_ to drive mute/volume hardware.
typedef struct {
    bool mute;
    uint8_t volume;  // 0-100
} audioDACdata_t;

namespace audio {

/**
 * Minimal AudioOutput adapter.
 * Wraps a pre-initialised i2s_chan_handle_t provided by SnapspotComponent.
 * Volume / mute stubs are intentionally simple — wire them to the DAC queue
 * or a hardware GPIO later if needed.
 */
class AudioOutput {
public:
    explicit AudioOutput(i2s_chan_handle_t tx_chan, uint32_t sample_rate, uint8_t bit_depth)
        : tx_chan_(tx_chan)
        , sample_rate_(sample_rate)
        , bit_depth_(bit_depth)
        , channel_enabled_(false)
        , muted_(false)
        , volume_(75)
    {}

    ~AudioOutput() = default;

    // ----------------------------------------------------------------
    // Lifecycle
    // ----------------------------------------------------------------

    /**
     * "Re-initialize" — channel is already set up by SnapspotComponent.
     * If the sample rate has changed, reconfigure the I2S clock.
     * settings is always nullptr in our ESPHome context (guarded by callers).
     */
    esp_err_t initialize(const config::Settings* /*settings*/,
                         uint32_t sample_rate, uint8_t bit_depth) {
        if (tx_chan_ == nullptr) return ESP_ERR_INVALID_STATE;
        if (sample_rate != sample_rate_) {
            esp_err_t ret = changeSampleRate(sample_rate);
            if (ret != ESP_OK) return ret;
        }
        bit_depth_ = bit_depth;
        return ESP_OK;
    }

    esp_err_t start() {
        if (tx_chan_ == nullptr) return ESP_ERR_INVALID_STATE;
        if (!channel_enabled_) {
            esp_err_t ret = i2s_channel_enable(tx_chan_);
            if (ret == ESP_OK) {
                channel_enabled_ = true;
            } else if (ret == ESP_ERR_INVALID_STATE) {
                // "already enabled" — another source left it active, that is fine.
                // Mark as enabled so write() / stop() track state correctly.
                channel_enabled_ = true;
                ret = ESP_OK;
            }
            return ret;
        }
        return ESP_OK;
    }

    esp_err_t stop() {
        if (tx_chan_ == nullptr) return ESP_ERR_INVALID_STATE;
        if (channel_enabled_) {
            esp_err_t ret = i2s_channel_disable(tx_chan_);
            if (ret == ESP_OK) channel_enabled_ = false;
            return ret;
        }
        return ESP_OK;
    }

    /** Immediate stop — same as stop() in our context (no APLL). */
    esp_err_t stopImmediate() { return stop(); }

    // ----------------------------------------------------------------
    // Audio write
    // ----------------------------------------------------------------

    /**
     * Write PCM audio to I2S.
     * Auto-enables the channel if it was disabled (mirrors snapspot-esp32 behaviour).
     */
    esp_err_t write(const void* data, size_t size,
                    size_t* bytes_written, uint32_t timeout_ms) {
        if (tx_chan_ == nullptr) {
            if (bytes_written) *bytes_written = 0;
            return ESP_ERR_INVALID_STATE;
        }

        TickType_t ticks = (timeout_ms == 0)
            ? portMAX_DELAY
            : pdMS_TO_TICKS(timeout_ms);

        esp_err_t ret = i2s_channel_write(tx_chan_, data, size, bytes_written, ticks);

        // Auto-enable channel if it was disabled and retry once.
        // Only update channel_enabled_ if the enable actually succeeds —
        // a failed enable (e.g. another source owns the channel) must NOT
        // mark us as enabled, or start() will skip the real enable later.
        if (ret == ESP_ERR_INVALID_STATE) {
            if (i2s_channel_enable(tx_chan_) == ESP_OK) {
                channel_enabled_ = true;
            }
            ret = i2s_channel_write(tx_chan_, data, size, bytes_written, ticks);
        }

        return ret;
    }

    // ----------------------------------------------------------------
    // Volume / mute  (simple stubs — wire to DAC queue if needed)
    // ----------------------------------------------------------------

    esp_err_t setVolume(uint8_t volume) {
        volume_ = volume;
        if (dac_queue_ != nullptr) {
            audioDACdata_t d = {muted_, volume_};
            xQueueOverwrite(dac_queue_, &d);
        }
        return ESP_OK;
    }

    uint8_t getVolume() const { return volume_; }

    esp_err_t setMute(bool muted) {
        muted_ = muted;
        if (dac_queue_ != nullptr) {
            audioDACdata_t d = {muted_, volume_};
            xQueueOverwrite(dac_queue_, &d);
        }
        return ESP_OK;
    }

    // ----------------------------------------------------------------
    // Sample-rate change
    // ----------------------------------------------------------------

    /**
     * Reconfigure I2S clock without tearing down the channel.
     * Uses i2s_channel_reconfig_std_clock() — IDF 5.x, no APLL.
     */
    esp_err_t changeSampleRate(uint32_t new_sample_rate) {
        if (tx_chan_ == nullptr) return ESP_ERR_INVALID_STATE;
        if (new_sample_rate == sample_rate_) return ESP_OK;

        // IDF 5.x requires the channel to be DISABLED before reconfiguring the clock.
        // Disable if currently active, then re-enable afterwards.
        bool was_enabled = channel_enabled_;
        if (was_enabled) {
            esp_err_t ret = i2s_channel_disable(tx_chan_);
            if (ret != ESP_OK) return ret;
            channel_enabled_ = false;
        }

        i2s_std_clk_config_t clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(new_sample_rate);
        esp_err_t ret = i2s_channel_reconfig_std_clock(tx_chan_, &clk_cfg);
        if (ret == ESP_OK) {
            sample_rate_ = new_sample_rate;
        }

        if (was_enabled) {
            esp_err_t en_ret = i2s_channel_enable(tx_chan_);
            if (en_ret == ESP_OK) channel_enabled_ = true;
            if (ret == ESP_OK) ret = en_ret;  // propagate enable error if reconfig was OK
        }
        return ret;
    }

    // ----------------------------------------------------------------
    // Accessors
    // ----------------------------------------------------------------

    bool isInitialized() const { return tx_chan_ != nullptr; }

    /** Always nullptr — both call-sites in snapcast_client.cpp guard with if (settings). */
    const config::Settings* getSettings() const { return nullptr; }

    uint32_t getSampleRate() const { return sample_rate_; }
    uint8_t getBitDepth() const { return bit_depth_; }

    /** No-op — source arbitration is handled by AudioSourceManager. */
    void setActiveSource(uint8_t /*source_id*/) {}

    /** Wire to SnapSpotComponent::audio_q_hdl_ so setMute/setVolume reach the DAC hardware. */
    void setDACQueue(QueueHandle_t q) { dac_queue_ = q; }

private:
    i2s_chan_handle_t tx_chan_;
    uint32_t sample_rate_;
    uint8_t bit_depth_;
    bool channel_enabled_;
    bool muted_;
    uint8_t volume_;
    QueueHandle_t dac_queue_{nullptr};
};

} // namespace audio
} // namespace snapspot
} // namespace esphome
