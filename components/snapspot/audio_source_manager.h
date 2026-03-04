/**
 * Audio Source Manager
 * 
 * Manages switching between multiple audio sources (Snapcast and Spotify)
 * with manual source selection. Ensures only one source writes to I2S
 * at a time to prevent driver conflicts.
 * 
 * Thread-Safety:
 * - All methods are thread-safe (mutex protected)
 * - Safe to call from multiple FreeRTOS tasks
 */

#pragma once

#include "esp_err.h"
#include "esp_log.h"  // For ESP_LOGI in inline methods
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include <cstdint>
#include <functional>

namespace esphome {
namespace snapspot {

// Forward declarations (audio_output handled by ESPHome i2s_audio + audio_dac)

namespace sources {

/**
 * Audio source state
 */
enum class SourceState : uint8_t {
    IDLE = 0,       // Source is inactive
    ACTIVE = 1,     // Source is actively playing
    PAUSED = 2,     // Source is paused by manager
    STOPPING = 3    // Source is stopping
};

/**
 * Audio source identifier
 */
enum class SourceId : uint8_t {
    NONE = 0,
    SNAPCAST = 1,
    SPOTIFY = 2
};

/**
 * Audio source interface
 * Each audio source (Snapcast, Spotify) must implement this interface
 */
class AudioSource {
public:
    virtual ~AudioSource() = default;
    
    /**
     * Get source identifier
     */
    virtual SourceId getId() const = 0;
    
    /**
     * Get current source state
     */
    virtual SourceState getState() const = 0;
    
    /**
     * Request to start playback
     * Called by manager when source is granted access
     */
    virtual esp_err_t requestStart() = 0;
    
    /**
     * Request to pause playback
     * Source must stop I2S writes immediately
     * @param force If true, stop immediately without cleanup
     */
    virtual esp_err_t requestPause(bool force) = 0;
    
    /**
     * Request to resume playback
     * Called when source regains access after pause
     */
    virtual esp_err_t requestResume() = 0;
    
    /**
     * Request to fully stop and disconnect
     * Called when switching to high-priority source to free CPU/RAM resources
     * More aggressive than requestPause() - should disconnect from servers,
     * stop background tasks (e.g., Mercury calls), and flush buffers
     */
    virtual esp_err_t requestStop() = 0;

    /**
     * Step 3 of staged switch: immediately stop all I2S writes without flushing.
     * Must return quickly (~0ms). Sets internal flags so the playback task drops
     * the current write on its next loop iteration.
     * No buffer flushing happens here — that is done by flushBuffers() after a delay.
     */
    virtual esp_err_t requestBlackHole() = 0;

    /**
     * Step 5 of staged switch: drain all software audio buffers.
     * Called while the source is already black-holed (no writes happening).
     * Must flush: ring buffer / audio queue / decoder / any intermediate buffer.
     * Does NOT touch I2S DMA — that is flushed by AudioOutput::flushDMA().
     */
    virtual esp_err_t flushBuffers() = 0;

    /**
     * Check if source has pending audio data
     * @return true if source wants to play audio
     */
    virtual bool wantsToPlay() const = 0;
};

/**
 * Audio Source Manager
 * Arbitrates access to AudioOutput between multiple sources
 */
class AudioSourceManager {
public:
    /**
     * Constructor
     * @param audio_output Shared audio output instance (must outlive manager)
     */
    explicit AudioSourceManager();
    ~AudioSourceManager();
    
    /**
     * Initialize manager
     */
    esp_err_t initialize();
    
    /**
     * Register an audio source
     * @param source Audio source instance (must outlive manager)
     */
    esp_err_t registerSource(AudioSource* source);
    
    /**
     * Unregister an audio source
     */
    esp_err_t unregisterSource(SourceId id);
    
    /**
     * Notify manager that a source wants to start playing
        * Manager will enforce current manual selection and grant/deny access
     * 
     * @param id Source requesting access
     * @return ESP_OK if granted, ESP_ERR_INVALID_STATE if denied
     */
    esp_err_t notifySourceActive(SourceId id);
    
    /**
     * Request exclusive access to audio output for a source
     * Same as notifySourceActive() but more explicit naming
     * Automatically handles DAC muting to prevent "click" noise
     * 
     * @param id Source requesting access
     * @return ESP_OK if granted, ESP_ERR_INVALID_STATE if denied
     */
    esp_err_t requestAccess(SourceId id) { return notifySourceActive(id); }
    
    /**
     * Notify manager that a source has stopped playing
     * Manager may activate another waiting source
     * 
     * @param id Source that stopped
     */
    esp_err_t notifySourceIdle(SourceId id);
    
    /**
     * Get currently active source
     */
    SourceId getActiveSource() const;

    /**
     * True while staged switch sequence is in progress.
     */
    bool isSwitchInProgress() const;
    
    /**
     * Check if a specific source is allowed to write to I2S
     * Call this before every write() to AudioOutput
     * 
     * @param id Source requesting permission
     * @return true if source can write
     */
    bool canSourceWrite(SourceId id) const;
    
    /**
     * Force a specific source to become active (for testing/debug)
     * WARNING: This bypasses priority system
     */
    esp_err_t forceActivateSource(SourceId id);
    
    /**
     * Get the mute delay used during source transitions (milliseconds)
     * This prevents "click" noise when switching between sources
     */
    uint32_t getSourceSwitchMuteDelayMs() const { return source_switch_mute_delay_ms_; }
    
    /**
     * Set the mute delay for source transitions
     * DAC is muted for this duration before/after switching
     * 
     * @param delay_ms Delay in milliseconds (default: 50ms)
     */
    void setSourceSwitchMuteDelayMs(uint32_t delay_ms);

    /**
     * Set manual source selection mode.
     * When set to SNAPCAST or SPOTIFY, automatic priority switching is disabled.
     * Use NONE to return to automatic behavior.
     */
    esp_err_t setManualSource(SourceId id);

    /**
     * Get currently forced manual source (NONE = automatic mode)
     */
    SourceId getManualSource() const { return manual_source_; }

    
    /**
     * Update Snapcast volume from server
     * Called by SnapcastClient when server sends volume command
     * @param volume Volume percentage (0-100)
     */
    void setSnapcastVolume(uint8_t volume);
    
    /**
     * Update Spotify volume
     * Called by SpotifyClient when user changes volume
     * @param volume Volume percentage (0-100)
     */
    void setSpotifyVolume(uint8_t volume);

    uint8_t getSnapcastVolume() const { return snapcast_volume_; }
    uint8_t getSpotifyVolume() const { return spotify_volume_; }

    /**
     * Called by SpotifyClient on PLAYBACK_START.
     * Sets a 5s hold window so a spurious DISC right after Play does not
     * let Snapcast immediately reclaim I2S.
     */
    void onSpotifyPlaybackStart();

    /**
     * Extend the active disc hold window by additional milliseconds.
     * Used at setCredentials() to cover auth + track-load time.
     */
    void extendDiscHold(uint32_t extra_ms);

    /**
     * Called by SpotifyClient on DISC.
     * If within hold window (set at PLAYBACK_START/setCredentials): keep hold,
     * Snapcast stays blocked, and the caller should NOT call notifySourceIdle
     * (cspot has no track after DISC — wait for PLAYBACK_START instead).
     * Returns true if within hold (caller should suppress notifySourceIdle).
     * Returns false if outside hold (caller should call notifySourceIdle normally).
     */
    bool onSpotifyDisc();

private:
    /**
     * Evaluate priority and switch sources if needed
     * Must be called with mutex locked
     * 
     * @param requesting_source ID of source requesting access (NONE if periodic check)
     */
    void evaluatePriority(SourceId requesting_source = SourceId::NONE);
    
    /**
     * Activate a source (internal)
     * Must be called with mutex locked
     */
    esp_err_t activateSource(SourceId id);
    
    /**
     * Pause a source (internal)
     * Must be called with mutex locked
     */
    esp_err_t pauseSource(SourceId id, bool force);
    
    /**
     * Fully stop a source (internal)
     * Calls requestStop() to disconnect and free resources
     * Must be called with mutex locked
     */
    esp_err_t stopSource(SourceId id);

    /**
     * Switch source using staged anti-click sequence:
     * mute -> delay -> flush old source -> delay -> activate new source -> delay -> unmute
     * Must be called with mutex locked
     */
    esp_err_t switchSourceStaged(SourceId desired);
    
    /**
     * Find source by ID
     * Must be called with mutex locked
     */
    AudioSource* findSource(SourceId id) const;

private:
    // audio_output_ removed: ESPHome handles mute/volume via audio_dac component
    
    AudioSource* sources_[3];               // Registered sources (max 3)
    uint8_t source_count_;                  // Number of registered sources
    
    SourceId active_source_;                // Currently active source
    SourceId pending_source_;               // Transition target allowed to write during staged switch
    
    // ***SEPARATE VOLUME MEMORIES*** (User requirement: don't trust AudioOutput)
    uint8_t snapcast_volume_;               // Snapcast's volume (default 80%)
    uint8_t spotify_volume_;                // Spotify's volume (default 30%)
    
    SemaphoreHandle_t mutex_;               // Protects all state
    
    uint32_t source_switch_mute_delay_ms_;  // Mute delay during source switch (default 50ms)
    uint32_t last_source_switch_time_ms_;   // Timestamp of last source switch (anti-flicker)
    uint32_t source_switch_hysteresis_ms_;  // Minimum time between switches (default 500ms)
    uint32_t spotify_disc_hold_until_ms_;   // Block Snapcast takeover until this time (set on DISC)
    SourceId manual_source_;                // NONE=auto, otherwise forced source

    bool initialized_;
    TaskHandle_t monitor_task_handle_;      // Periodic priority re-evaluation task

    // Periodic task: re-evaluates source priority every 1s so the disc hold
    // expiry is acted on even when no other event fires evaluatePriority().
    static void monitorTask(void* pvParameters);
};

} // namespace sources
} // namespace snapspot
} // namespace esphome
