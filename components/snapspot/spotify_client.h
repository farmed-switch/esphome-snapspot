/**
 * Spotify Client Wrapper
 * 
 * Wrapper around cspot library for ESP-IDF integration.
 * Implements AudioSource interface for AudioSourceManager.
 * 
 * Key Features:
 * - Spotify Connect (Zeroconf discovery)
 * - PSRAM-based audio buffers (same as Snapcast)
 * - Vorbis decode (44.1kHz/16-bit locked)
 * - Priority-based switching via AudioSourceManager
 */

#pragma once

#include "esp_err.h"
#include "audio_source_manager.h"
#include "driver/i2s_std.h"
#include "esphome/components/audio_dac/audio_dac.h"
#include <memory>
#include <cstdint>
#include <string>

// Forward declarations for cspot types (avoid including cspot headers here)
namespace cspot {
struct Context;
class SpircHandler;
class TrackPlayer;
class LoginBlob;
}

namespace bell {
class CircularBuffer;
}

namespace esphome {
namespace snapspot {
namespace spotify {

/**
 * Spotify Client
 * Wraps cspot library and integrates with AudioSourceManager
 */
class SpotifyClient : public sources::AudioSource {
public:
    /**
     * Constructor
     * @param source_manager Audio source manager for priority arbitration
     * @param device_name  Spotify device name shown in the app
     * @param tx_chan       I2S TX channel handle (opened by SnapSpotComponent)
     * @param audio_dac    Optional DAC for volume/mute control
     */
    SpotifyClient(sources::AudioSourceManager* source_manager, std::string device_name,
                  i2s_chan_handle_t tx_chan, esphome::audio_dac::AudioDac* audio_dac = nullptr);
    ~SpotifyClient();
    
    /**
     * Initialize Spotify Connect
     * Sets up cspot context, mDNS, and credential storage
     */
    esp_err_t initialize();
    
    /**
     * Start Spotify Connect service
     * Device will appear in Spotify app
     */
    esp_err_t start();
    
    /**
     * Stop Spotify Connect service
     */
    esp_err_t stop();
    
    /**
     * Check if enabled in config
     */
    bool isEnabled() const;
    
    /**
     * Check if currently playing
     */
    bool isPlaying() const;
    
    /**
     * Get device name (for mDNS/Spotify UI)
     */
    const char* getDeviceName() const;
    
    /**
     * Set Spotify credentials from Zeroconf authentication
     * Called by web handler when Spotify app sends credentials
     * @param username Spotify username
     * @param blob Authentication blob (encrypted credentials)
     * @param client_key Client key for authentication
     * @return ESP_OK on success
     */
    esp_err_t setCredentials(const char* username, const char* blob, const char* client_key);
    
    /**
     * Check if credentials are stored
     */
    bool hasCredentials() const;

    // ========================================================================
    // Track Metadata (updated on TRACK_INFO cspot event)
    // ========================================================================
    const char* getCurrentTrack() const   { return current_track_name_.c_str(); }
    const char* getCurrentArtist() const  { return current_artist_.c_str(); }
    const char* getCurrentAlbum() const   { return current_album_.c_str(); }
    const char* getCurrentImageUrl() const { return current_image_url_.c_str(); }
    uint32_t    getCurrentDurationMs() const { return current_duration_ms_; }
    uint64_t    getCurrentPositionMs() const;  // estimated from bytes played
    
    /**
     * Get Zeroconf info JSON (contains publicKey for Spotify app)
     */
    std::string getZeroconfInfo() const;

    // Debug control helpers (for web debug panel)
    esp_err_t debugConnect();
    esp_err_t debugDisconnect();
    esp_err_t debugPlay();
    esp_err_t debugPause();
    esp_err_t debugStop();

    // Configure the dB range used when translating Spotify volume 0-100% to set_volume().
    // spotify_min/max_db: the dB range Spotify occupies (e.g. -25..0 limits max to 0dBFS).
    // dac_min/max_db: TAS5805M volume_min/volume_max (must match YAML, default -25..15).
    void set_volume_db_range(float spotify_min, float spotify_max,
                             float dac_min, float dac_max) {
        spotify_min_db_ = spotify_min;
        spotify_max_db_ = spotify_max;
        dac_min_db_     = dac_min;
        dac_max_db_     = dac_max;
    }

    // Scale Spotify 0-100% to 0.0-1.0 respecting source dB window vs DAC range.
    float scaleSpotifyVolume(float percent) const {
        float dac_range = dac_max_db_ - dac_min_db_;
        if (dac_range <= 0.0f) return percent / 100.0f;
        float target_db = spotify_min_db_ + (spotify_max_db_ - spotify_min_db_) * (percent / 100.0f);
        float v = (target_db - dac_min_db_) / dac_range;
        if (v < 0.0f) v = 0.0f;
        if (v > 1.0f) v = 1.0f;
        return v;
    }

    // ========================================================================
    // AudioSource Interface Implementation
    // ========================================================================
    
    sources::SourceId getId() const override {
        return sources::SourceId::SPOTIFY;
    }
    
    // AudioSource interface (implemented by Spotify to integrate with AudioSourceManager)
    sources::SourceState getState() const override;
    bool wantsToPlay() const override;  // Returns true when playing with audio data
    esp_err_t requestStart() override;   // Called when manager grants us access
    esp_err_t requestPause(bool force) override; // Called when manager pauses this source
    esp_err_t requestResume() override;  // Called when we can resume after pause
    esp_err_t requestStop() override;    // Soft stop: stop local playback, keep session alive
    esp_err_t requestBlackHole() override; // Step 3: stop I2S writes immediately, no flush
    esp_err_t flushBuffers() override;     // Step 5: drain ring buffer + decoder

private:
    /**
     * cspot data callback (called when decoded audio is ready)
     * Runs in cspot's player task context
     */
    void feedAudioData(uint8_t* data, size_t bytes);
    
    /**
     * cspot event handler (play/pause/seek/track change)
     */
    void handleCSpotEvent(int event_type);
    
    /**
     * Audio playback task (reads from ring buffer, writes to I2S)
     * Similar to Snapcast's PlaybackTask
     */
    static void playbackTask(void* pvParameters);
    
    /**
     * Packet processing task (receives CDN chunks, decodes audio)
     * HIGH PRIORITY (20) - runs handlePacket() continuously
     */
    static void packetProcessingTask(void* pvParameters);

    /**
     * Emit a very short ramp-to-zero on pause to avoid loud pop/click.
     */
    void emitPauseFadeOut();

    /**
     * Request AudioSourceManager ownership when needed, with light debounce
     * to avoid duplicate startup/resume access storms.
     */
    void requestSourceAccessIfNeeded(const char* reason);
    void notifySourceIdleAsync(const char* reason);

    static void sourceManagerActionTask(void* pvParameters);
    
    /**
     * Check if AudioSourceManager allows us to write to I2S
     */
    bool canWriteToAudioOutput() const;

private:
    sources::AudioSourceManager* source_manager_;  // Audio arbiter (not owned)
    std::string device_name_;                    // Spotify device name
    i2s_chan_handle_t tx_chan_;                  // I2S TX channel (not owned)
    esphome::audio_dac::AudioDac* audio_dac_;   // DAC for volume/mute (not owned; may be nullptr)
    
    // cspot core objects (forward-declared, created in .cpp)
    std::shared_ptr<cspot::LoginBlob> login_blob_;      // For Zeroconf publicKey generation
    std::shared_ptr<cspot::Context> cspot_context_;
    std::shared_ptr<cspot::SpircHandler> spirc_handler_;
    
    // Audio buffer (PSRAM-allocated ring buffer)
    // NOTE: Using raw pointer for forward-declared type (unique_ptr requires complete type)
    bell::CircularBuffer* audio_buffer_;
    
    // Spotify credentials (from Zeroconf authentication)
    std::string username_;
    std::string auth_blob_;
    std::string client_key_;
    bool has_credentials_;
    
    // State tracking
    sources::SourceState state_;
    bool is_playing_;
    bool is_paused_by_manager_;  // Paused by AudioSourceManager (Snapcast priority)
    bool is_paused_by_app_;      // Paused by user in Spotify app (blocks buffer-driven auto-resume)
    bool dac_unmute_pending_;    // Set by requestStart(); cleared after DMA prime in playbackTask
    bool dma_ring_is_clean_;     // True after fade-out (ring has silence) → skip DMA prime on resume

    // Playback timing control (prevents bursty/fast playback)
    bool playback_clock_initialized_;   // True when local playback clock is running
    bool playback_clock_reset_pending_; // Set on seek/flush/source switch
    uint64_t playback_clock_base_us_;   // Local time when byte position 0 starts playing
    uint64_t playback_bytes_played_;    // Bytes successfully written to I2S since base
    uint32_t spotify_prebuffer_ms_;     // Initial/rebuffer target (default 1000 ms)

    // Last decoded stereo sample for pause fade-out (pop suppression)
    int16_t last_left_sample_;
    int16_t last_right_sample_;
    bool last_sample_valid_;
    uint64_t last_access_request_us_;
    uint64_t last_playback_start_us_;
    uint64_t accept_audio_after_us_;
    
    // Current track metadata (written on TRACK_INFO event under state_mutex_)
    std::string  current_track_name_;
    std::string  current_artist_;
    std::string  current_album_;
    std::string  current_image_url_;
    uint32_t     current_duration_ms_ = 0;

    // Volume dB-range for this source (set from YAML, applied to every set_volume() call)
    float spotify_min_db_{-25.0f};
    float spotify_max_db_{15.0f};
    float dac_min_db_{-25.0f};
    float dac_max_db_{15.0f};

    // FreeRTOS tasks
    TaskHandle_t playback_task_handle_;   // Playback task (writes to I2S, priority 10)
    TaskHandle_t packet_task_handle_;     // Packet processing task (CDN chunks, priority 20)
    TaskHandle_t auth_task_handle_;       // Background authentication task
    
    // Thread safety
    SemaphoreHandle_t state_mutex_;
    
    // Private methods
    static void authenticationTask(void* pvParameters);
};

} // namespace spotify
} // namespace snapspot
} // namespace esphome
