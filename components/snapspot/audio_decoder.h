/**
 * Audio Decoder Interface
 * Abstracts FLAC, Opus, PCM decoding
 * Ported from snapspot-esp32/main/audio_decoder.h
 */

#pragma once

#include "esp_err.h"
#include <cstdint>
#include <cstddef>

namespace esphome {
namespace snapspot {
namespace audio {

enum class CodecType {
    PCM,
    FLAC,
    OPUS,
    UNKNOWN
};

class AudioDecoder {
public:
    virtual ~AudioDecoder() = default;

    // Initialize decoder with codec-specific config
    virtual esp_err_t init(const uint8_t* codec_header, size_t header_size) = 0;

    // Decode compressed audio chunk to PCM
    // Returns number of PCM bytes written to out_buffer
    virtual int decode(const uint8_t* in_data, size_t in_size,
                       uint8_t* out_buffer, size_t out_buffer_size) = 0;

    // Get audio format after decoding
    virtual uint32_t getSampleRate() const = 0;
    virtual uint16_t getBitDepth() const = 0;
    virtual uint16_t getChannels() const = 0;

    // Reset decoder state
    virtual void reset() = 0;
};

// Factory function
AudioDecoder* createDecoder(CodecType codec);

} // namespace audio
} // namespace snapspot
} // namespace esphome
