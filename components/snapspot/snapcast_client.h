/**
 * Snapcast Client
 * Connects to Snapcast server and receives audio stream
 * Ported from snapspot-esp32/main/snapcast_client.h (namespace wrapped for ESPHome)
 */

#pragma once

#include "esp_err.h"
#include "audio_output.h"
#include "audio_decoder.h"
#include "audio_source_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include <cstdint>

namespace esphome {
namespace snapspot {

// Forward declaration — dsp_processor is not available in ESPHome context.
// DspProcessor pointer is always nullptr; all call-sites are guarded by if (client->dsp_).
namespace dsp { class DspProcessor; }

namespace snapcast {

// Snapcast ports
constexpr uint16_t DEFAULT_STREAM_PORT = 1704;
constexpr uint16_t DEFAULT_CONTROL_PORT = 1705;

// Buffer sizes
constexpr size_t AUDIO_BUFFER_SIZE = 64 * 1024;  // 64KB in PSRAM
constexpr size_t CHUNK_BUFFER_SIZE = 8 * 1024;   // 8KB for single chunk

// Queue configuration for task separation
// Queue holds audio chunks between NetworkRx and Playback tasks
// 500 chunks @ 20ms/chunk = 10s buffering, ~2MB PSRAM (chunks dynamically alloc'd)
// Large buffer absorbs network jitter and Snapcast server BufferMs margins
constexpr size_t AUDIO_QUEUE_LENGTH = 500;  // ~10s @ 20ms/chunk, ~2MB PSRAM

/**
 * Audio chunk item for queue between tasks
 * NetworkRxTask receives COMPRESSED chunks and pushes to queue
 * PlaybackTask pops, DECODES, and plays with timing
 *
 * CRITICAL: Decoding moved to PlaybackTask to prevent blocking recv()
 */
struct AudioChunkItem {
    int64_t server_timestamp_us;     // Server time (sec.usec converted to us)
    uint32_t compressed_size;        // Size of compressed data in bytes
    uint8_t* compressed_data;        // Dynamically allocated compressed buffer (PSRAM)
};

enum class ClientState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    STREAMING,
    ERROR
};

class SnapcastClient : public sources::AudioSource {
public:
    SnapcastClient(audio::AudioOutput* audio_output, sources::AudioSourceManager* source_manager);
    ~SnapcastClient();

    /**
     * Set server hostname and port, then initiate connection.
     * Non-blocking — starts background tasks.
     */
    esp_err_t connect(const char* hostname, uint16_t port = DEFAULT_STREAM_PORT);

    /**
     * Disconnect from server and stop background tasks.
     */
    void disconnect();

    /**
     * Start streaming (call after connect() succeeds).
     */
    esp_err_t start();

    /**
     * Stop streaming.
     */
    void stop();

    // ----------------------------------------------------------------
    // AudioSource pure virtual overrides
    // ----------------------------------------------------------------
    sources::SourceId getId() const override { return sources::SourceId::SNAPCAST; }
    sources::SourceState getState() const override;
    esp_err_t requestStart() override;
    esp_err_t requestPause(bool force) override;
    esp_err_t requestResume() override;
    esp_err_t requestStop() override;
    esp_err_t requestBlackHole() override;
    esp_err_t flushBuffers() override;

    /** Get internal Snapcast-protocol state (not the AudioSource state). */
    ClientState getClientState() const { return state_; }

    /** Get approximate buffer fill level (0-100%).
     *  Used by wantsToPlay() — also called externally for diagnostics.
     */
    uint8_t getBufferLevel() const;

    /**
     * Reset time synchronization state.
     * Call when switching from another source to Snapcast to clear old drift data.
     */
    void resetTimeSync();

    /**
     * Flush audio queue (discard all pending chunks)
     * Call when switching sources to prevent playing old buffered chunks.
     * This prevents hearing 200-500ms delayed audio from chunks queued before source switch.
     */
    void flushAudioQueue();

    /**
     * Report local volume/mute change back to Snapcast server.
     * Call this when the user changes volume via IR remote or web UI
     * so the server stays in sync with the client's actual state.
     * No-op if not connected (sock_ < 0).
     */
    esp_err_t sendVolumeNotification(uint8_t volume_percent, bool muted);

    bool wantsToPlay() const override {
        // Snapcast wants to play ONLY if it has meaningful audio data buffered
        // AND has actually written to I2S recently (within last 5 seconds)
        // AND is not in an unhealthy state (frequent queue flushes)
        //
        // This prevents Snapcast from blocking Spotify when:
        // - User has switched to another source in Snapcast app
        // - Server is still sending chunks but we're not actually playing
        // - Buffer fills up but audio isn't being consumed
        // - Chunks are repeatedly late (frequent flushes)

        if (state_ != ClientState::STREAMING) {
            return false;  // Not streaming → don't want to play
        }

        if (getBufferLevel() <= 5) {
            return false;  // Buffer too low → don't want to play
        }

        uint32_t now_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

        // Check if we've flushed recently (within last 3 seconds)
        // Frequent flushes indicate unhealthy playback (stale/late chunks)
        uint32_t time_since_last_flush = now_ms - last_queue_flush_time_ms_;
        if (time_since_last_flush < 3000) {
            // Recent flush → we're not in stable playback, release priority!
            return false;
        }

        // Check if we've written to I2S recently (within last 5 seconds)
        uint32_t time_since_last_write = now_ms - last_i2s_write_time_ms_;
        if (time_since_last_write > 5000) {
            // No I2S writes for 5+ seconds → we're idle, let Spotify play!
            return false;
        }

        return true;  // Buffer full AND recently active AND healthy → we want to play!
    }

    /**
     * Set DSP processor (call after init, before streaming starts).
     * NULL is safe — DSP will simply be skipped.
     */
    void setDsp(dsp::DspProcessor* dsp) { dsp_ = dsp; }

private:
    /**
     * Network RX task - receives packets from socket (HIGH PRIORITY)
     * This task ONLY does recv() and captures t4 timestamp immediately
     * Audio chunks are pushed to queue, TIME sync processed directly
     */
    static void networkRxTask(void* pvParameters);

    /**
     * Playback task - pops chunks from queue and plays with timing
     * This task handles vTaskDelay() for precise timing without blocking network
     */
    static void playbackTask(void* pvParameters);

    /**
     * Helper: recv() exact number of bytes with loop guarantee
     * Ensures alignment by reading exactly the requested size
     */
    esp_err_t recvExact(int sock, void* buffer, size_t size);

    /**
     * Receive Snapcast chunk from network
     */
    esp_err_t receiveChunk();

    /**
     * Send Hello message to server (required handshake)
     */
    esp_err_t sendHello();

    /**
     * Handle codec header from server
     */
    esp_err_t handleCodecHeader(uint32_t payload_size);

    /**
     * Parse and handle Snapcast message
     */
    esp_err_t handleMessage(const uint8_t* data, size_t len);

    /**
     * Handle TIME synchronization message (NTP-style)
     * @param t4_timestamp_us Client receive timestamp (microseconds), must be captured immediately after recv()
     */
    esp_err_t handleTimeMessage(int32_t server_sent_sec, int32_t server_sent_usec,
                                int32_t server_recv_sec, int32_t server_recv_usec,
                                int64_t t4_timestamp_us);

    /**
     * Handle ServerSettings JSON from server
     */
    esp_err_t handleServerSettings(const char* json_payload, size_t payload_size);

    /**
     * Handle JSON-RPC control messages (volume, mute, etc.)
     */
    esp_err_t handleJsonRpcMessage(const char* json_payload, size_t payload_size);

    /**
     * Send TIME request to server for clock synchronization
     * Client must periodically send TIME messages to get server time sync
     */
    esp_err_t sendTimeRequest();

    /**
     * Initialize SNTP client for network time synchronization
     */
    void initSNTP();

    audio::AudioOutput* audio_output_;
    sources::AudioSourceManager* source_manager_;  // Audio arbiter (not owned)
    bool audio_access_granted_;  // True when AudioSourceManager has granted us I2S access
    audio::AudioDecoder* decoder_;
    dsp::DspProcessor* dsp_;  // Optional EQ/DSP processor (not owned; nullptr = bypass)
    int sock_;
    ClientState state_;

    // Task separation: Network and Playback run independently
    TaskHandle_t network_task_handle_;  // High priority network RX
    TaskHandle_t playback_task_handle_; // Medium priority playback with timing
    QueueHandle_t audio_queue_;         // Queue from network → playback
    bool running_;

    uint32_t bytes_received_;
    uint32_t chunks_received_;

    // Audio format from server
    uint32_t sample_rate_;
    uint8_t bit_depth_;
    uint8_t channels_;
    bool format_received_;

    char hostname_[64];
    uint16_t port_;

    // ============================================================
    // Time Synchronization (Snapcast NTP-style protocol)
    // ============================================================

    // Server time offset in microseconds (μs)
    // Formula: local_time + server_offset_us = server_time
    int64_t server_offset_us_;
    bool time_sync_initialized_;  // True after first TIME message or chunk-based init

    // Moving median filter for RTT/offset measurements (reduces network jitter)
    // ESPHome/Snapclient uses 50 samples for stable time sync
    static constexpr size_t TIME_SAMPLES = 50;
    int64_t offset_samples_[TIME_SAMPLES];
    size_t offset_sample_index_;
    size_t offset_samples_count_;

    // RTT statistics
    int64_t last_rtt_us_;
    int64_t min_rtt_us_;

    // TIME request tracking (for NTP calculation)
    int64_t last_time_request_sent_us_;  // Our local time when we sent TIME request (t1)

    // SNTP sync status
    bool sntp_initialized_;
    bool sntp_synced_;

    // ============================================================
    // Server Settings & Volume Control
    // ============================================================

    // Server-provided buffering parameters
    uint32_t buffer_ms_;        // Buffer size expected by server (e.g., 1000ms)
    uint32_t latency_ms_;       // Server's internal latency
    uint32_t client_dac_latency_ms_;  // Client's DAC hardware latency (default 0)

    // Current volume/mute state
    uint8_t volume_percent_;    // 0-100
    bool muted_;

    // First packet unmute flag (reset on each activation)
    bool first_packet_unmuted_;  // True after first successful write + unmute
    uint8_t unmute_guard_writes_; // Require N successful writes before unmuting DAC

    // ***IDLE TIMEOUT***: Track last I2S write time
    // If no I2S writes for 5 seconds, wantsToPlay() returns FALSE
    // This prevents Snapcast from blocking Spotify when user has switched sources
    uint32_t last_i2s_write_time_ms_;  // Last successful I2S write timestamp

    // ***FLUSH MONITORING***: Track queue flushes (late chunks)
    // If we flush frequently, we're not in healthy playback state
    // This catches cases where server sends stale data but we still write to I2S occasionally
    uint32_t last_queue_flush_time_ms_;  // When we last flushed the audio queue

    // JSON-RPC message ID counter (for correlation)
    uint16_t jsonrpc_id_counter_;
};

} // namespace snapcast
} // namespace snapspot
} // namespace esphome
