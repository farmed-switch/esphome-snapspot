#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * Initialize (or reinitialize) the 18-band graphic EQ.
 *
 * Computes biquad coefficients from the current gain_db[] values.
 * Must be called once before eq_process_stereo_int16(), and again
 * whenever the sample rate changes. Thread-safe via internal spinlock.
 *
 * @param sample_rate  Audio sample rate in Hz (e.g. 44100).
 */
void eq_init(uint32_t sample_rate);

/**
 * Switch between 15-band ISO mode and 18-band dense-bass mode.
 *
 * Resets all band gains to 0 dB and reinitialises coefficients.
 * 15-band: ISO 2/3-octave (25 Hz – 16 kHz), matching TAS5805M hardware DSP.
 * 18-band: dense bass bands (40–140 Hz) + upper range (200 Hz – 5 kHz).
 *
 * @param bands  15 or 18.
 */
void eq_set_mode(int bands);

/**
 * Process interleaved stereo int16 audio in-place through the EQ.
 *
 * No-op when EQ is disabled. Applies 18 cascaded peaking-EQ biquads plus
 * cubic soft-clipping at ±70% to prevent clipping artefacts.
 *
 * @param samples      Interleaved [L,R,L,R,...] buffer (modified in-place).
 * @param num_samples  Number of stereo FRAMES (not individual samples).
 */
void eq_process_stereo_int16(int16_t *samples, size_t num_samples);

/**
 * Set gain for a single band and immediately recompute coefficients.
 *
 * @param band    Band index 0-17 (40 Hz .. 5000 Hz).
 * @param gain_db Gain in dB, clamped to ±15.0.
 */
void set_eq_band(int band, float gain_db);

/**
 * Apply a named preset and immediately recompute coefficients.
 *
 * Supported names:
 *   "Flat (Bypass)"    — all 0 dB, EQ disabled (full bypass)
 *   "Flat (Full Range)"— all 0 dB, EQ enabled
 *   "Subwoofer"        — deep bass boost, steep high-frequency rolloff
 *   "Bookshelf"        — light bass boost for small speakers
 *   "Floor Standing"   — balanced curve for tower speakers
 *   "Near Field"       — studio monitor / neutral
 *   "Small/Portable"   — boosted mids/highs for laptop/phone speakers
 *   "Custom"           — enables EQ without changing gains
 *
 * @param preset  Null-terminated preset name (not written to).
 */
void apply_eq_preset(const char *preset);

/**
 * Enable or disable the entire EQ.
 *
 * Enabling clears the biquad delay lines to avoid audible clicks.
 *
 * @param enable  true = process audio, false = bypass.
 */
void enable_eq(bool enable);

/**
 * Driver-agnostic hook for hardware DSP EQ band control.
 *
 * Called by set_eq_band() whenever a slider changes, regardless of which
 * DSP mode is currently active. The default implementation is a no-op
 * (defined as a weak symbol). Override this in your device-specific
 * component/file to forward gains to the hardware DSP chip:
 *
 *   // Example for a TAS58xx or similar:
 *   void hw_dsp_set_band(int band, float gain_db) {
 *       id(tas5805m_dac).set_eq_band_gain(band, gain_db);
 *   }
 *
 * This keeps the eq-controls package driver-agnostic — it works with
 * any hardware DSP that can accept per-band gain updates.
 *
 * @param band     Band index (0 = lowest frequency).
 * @param gain_db  Gain in dB (same value as the slider).
 */
void hw_dsp_set_band(int band, float gain_db);

#ifdef __cplusplus
}
#endif
