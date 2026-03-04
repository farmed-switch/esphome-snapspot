#ifndef GRAPHIC_EQ_H
#define GRAPHIC_EQ_H

#include <cmath>
#include <string.h>
#include "esp_log.h"

// Define M_PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 18-band graphic equalizer (11 bass bands 40-140Hz + 7 higher bands)
#define EQ_BANDS 18

// EQ configuration
typedef struct {
    float center_freq[EQ_BANDS];     // Center frequencies for each band
    float q_factor[EQ_BANDS];         // Q factor for each band (bandwidth)
    float gain_db[EQ_BANDS];          // Gain in dB for each band
    float coeffs[EQ_BANDS][5];        // Biquad coefficients (b0, b1, b2, a1, a2)
    float delay_left[EQ_BANDS][2];    // Left channel delay lines (w0, w1)
    float delay_right[EQ_BANDS][2];   // Right channel delay lines (w0, w1)
    bool enabled;                     // Master EQ enable/disable
    uint32_t sample_rate;             // Sample rate (e.g., 44100)
} graphic_eq_t;

// Global EQ instance (defined in decoder.cpp)
extern graphic_eq_t eq;

#define EQ_TAG "GraphicEQ"

/**
 * Initialize the graphic equalizer
 * Call this after sample rate is known from metadata
 */
inline void eq_init(uint32_t sample_rate) {
    eq.sample_rate = sample_rate;
    ESP_LOGI(EQ_TAG, "Initializing 18-band graphic EQ @ %lu Hz", sample_rate);
    
    // Clear all delay lines
    memset(eq.delay_left, 0, sizeof(eq.delay_left));
    memset(eq.delay_right, 0, sizeof(eq.delay_right));
    
    // Generate coefficients for all bands
    for (int band = 0; band < EQ_BANDS; band++) {
        // Normalize frequency: f_normalized = f_hz / (sample_rate / 2)
        float norm_freq = eq.center_freq[band] / (eq.sample_rate / 2.0f);
        
        if (eq.gain_db[band] == 0.0f) {
            // Bypass: Unity gain, no filtering
            eq.coeffs[band][0] = 1.0f;  // b0
            eq.coeffs[band][1] = 0.0f;  // b1
            eq.coeffs[band][2] = 0.0f;  // b2
            eq.coeffs[band][3] = 0.0f;  // a1
            eq.coeffs[band][4] = 0.0f;  // a2
        } else {
            // Generate peaking EQ filter coefficients (manual biquad implementation)
            float gain_linear = powf(10.0f, eq.gain_db[band] / 20.0f);
            
            // Peaking EQ biquad coefficients (Audio EQ Cookbook)
            float alpha = sinf(2.0f * M_PI * norm_freq) / (2.0f * eq.q_factor[band]);
            float A = gain_linear;
            
            // Recalculate with gain
            eq.coeffs[band][0] = 1.0f + alpha * A;        // b0
            eq.coeffs[band][1] = -2.0f * cosf(2.0f * M_PI * norm_freq);  // b1
            eq.coeffs[band][2] = 1.0f - alpha * A;        // b2
            eq.coeffs[band][3] = -2.0f * cosf(2.0f * M_PI * norm_freq);  // a1 (same as b1)
            eq.coeffs[band][4] = 1.0f - alpha / A;        // a2
            
            // Normalize by a0 = (1 + alpha / A)
            float a0 = 1.0f + alpha / A;
            eq.coeffs[band][0] /= a0;
            eq.coeffs[band][1] /= a0;
            eq.coeffs[band][2] /= a0;
            eq.coeffs[band][3] /= a0;
            eq.coeffs[band][4] /= a0;
        }
        
        ESP_LOGI(EQ_TAG, "Band %d: %.0fHz, Q=%.1f, Gain=%.1fdB", 
                 band, eq.center_freq[band], eq.q_factor[band], eq.gain_db[band]);
    }
}

/**
 * Process stereo audio through the 10-band EQ
 * @param samples: Interleaved stereo int16 buffer [L, R, L, R, ...]
 * @param num_samples: Number of FRAMES (stereo pairs)
 */
inline void eq_process_stereo_int16(int16_t* samples, size_t num_samples) {
    if (!eq.enabled) {
        return;  // Bypass EQ
    }
    
    // Process each frame (stereo pair)
    for (size_t i = 0; i < num_samples; i++) {
        // Convert int16 to float [-1.0, 1.0]
        float left = samples[i * 2] / 32768.0f;
        float right = samples[i * 2 + 1] / 32768.0f;
        
        // Apply all 10 EQ bands in cascade (series)
        for (int band = 0; band < EQ_BANDS; band++) {
            // Left channel biquad
            float w_left = left - eq.coeffs[band][3] * eq.delay_left[band][0] 
                                 - eq.coeffs[band][4] * eq.delay_left[band][1];
            left = eq.coeffs[band][0] * w_left 
                 + eq.coeffs[band][1] * eq.delay_left[band][0] 
                 + eq.coeffs[band][2] * eq.delay_left[band][1];
            
            // Update left delay line
            eq.delay_left[band][1] = eq.delay_left[band][0];
            eq.delay_left[band][0] = w_left;
            
            // Right channel biquad
            float w_right = right - eq.coeffs[band][3] * eq.delay_right[band][0] 
                                  - eq.coeffs[band][4] * eq.delay_right[band][1];
            right = eq.coeffs[band][0] * w_right 
                  + eq.coeffs[band][1] * eq.delay_right[band][0] 
                  + eq.coeffs[band][2] * eq.delay_right[band][1];
            
            // Update right delay line
            eq.delay_right[band][1] = eq.delay_right[band][0];
            eq.delay_right[band][0] = w_right;
        }
        
        // Cubic soft-clipping to prevent overflow and reduce distortion
        // Uses a smooth cubic curve near clipping threshold (0.7) instead of hard clipping at 1.0
        // This prevents the "cracks" that occur with large EQ gains (Andy's cubic soft-clipping)
        const float threshold = 0.7f;  // Start soft-clipping at 70% of max
        
        // Left channel cubic soft-clip
        if (left > 1.0f) {
            left = 1.0f;  // Hard limit at absolute max
        } else if (left > threshold) {
            // Smooth cubic transition from threshold to 1.0
            float x = (left - threshold) / (1.0f - threshold);
            left = threshold + (1.0f - threshold) * (x - x*x*x/3.0f);
        } else if (left < -1.0f) {
            left = -1.0f;
        } else if (left < -threshold) {
            // Smooth cubic transition for negative side
            float x = (-left - threshold) / (1.0f - threshold);
            left = -threshold - (1.0f - threshold) * (x - x*x*x/3.0f);
        }
        
        // Right channel cubic soft-clip
        if (right > 1.0f) {
            right = 1.0f;
        } else if (right > threshold) {
            float x = (right - threshold) / (1.0f - threshold);
            right = threshold + (1.0f - threshold) * (x - x*x*x/3.0f);
        } else if (right < -1.0f) {
            right = -1.0f;
        } else if (right < -threshold) {
            float x = (-right - threshold) / (1.0f - threshold);
            right = -threshold - (1.0f - threshold) * (x - x*x*x/3.0f);
        }
        
        // Convert back to int16
        samples[i * 2] = (int16_t)(left * 32767.0f);
        samples[i * 2 + 1] = (int16_t)(right * 32767.0f);
    }
}

/**
 * ESPHome callback: Set individual EQ band gain
 * @param band: Band index (0-9)
 * @param gain_db: Gain in dB (-15 to +15)
 */
extern "C" inline void set_eq_band(int band, float gain_db) {
    if (band < 0 || band >= EQ_BANDS) {
        ESP_LOGE(EQ_TAG, "Invalid band index: %d", band);
        return;
    }
    
    if (gain_db < -15.0f || gain_db > 15.0f) {
        ESP_LOGE(EQ_TAG, "Invalid gain: %.1fdB (must be -15 to +15)", gain_db);
        return;
    }
    
    eq.gain_db[band] = gain_db;
    ESP_LOGI(EQ_TAG, "Band %d (%.0fHz) set to %.1fdB", band, eq.center_freq[band], gain_db);
    
    // Regenerate coefficients for this band
    eq_init(eq.sample_rate);
}

/**
 * ESPHome callback: Apply preset
 */
extern "C" inline void apply_eq_preset(const char* preset) {
    ESP_LOGI(EQ_TAG, "Applying preset: %s", preset);
    
    if (strcmp(preset, "Flat (Bypass)") == 0) {
        // All bands to 0dB, EQ disabled
        for (int i = 0; i < EQ_BANDS; i++) {
            eq.gain_db[i] = 0.0f;
        }
        eq.enabled = false;  // Full bypass
        
    } else if (strcmp(preset, "Flat (Full Range)") == 0) {
        // All bands to 0dB, EQ enabled (for transparent processing)
        for (int i = 0; i < EQ_BANDS; i++) {
            eq.gain_db[i] = 0.0f;
        }
        eq.enabled = true;
        
    } else if (strcmp(preset, "Subwoofer") == 0) {
        // Deep bass boost + lowpass filter (cut everything above 120Hz)
        eq.gain_db[0] = 8;     // 40Hz
        eq.gain_db[1] = 7;     // 50Hz
        eq.gain_db[2] = 6;     // 60Hz
        eq.gain_db[3] = 5;     // 70Hz
        eq.gain_db[4] = 4;     // 80Hz
        eq.gain_db[5] = 3;     // 90Hz
        eq.gain_db[6] = 2;     // 100Hz
        eq.gain_db[7] = 1;     // 110Hz
        eq.gain_db[8] = 0;     // 120Hz (crossover point)
        eq.gain_db[9] = -10;   // 130Hz (steep rolloff)
        eq.gain_db[10] = -12;  // 140Hz
        eq.gain_db[11] = -13;  // 200Hz
        eq.gain_db[12] = -14;  // 315Hz
        eq.gain_db[13] = -15;  // 500Hz (full cut)
        eq.gain_db[14] = -15;  // 800Hz
        eq.gain_db[15] = -15;  // 1250Hz
        eq.gain_db[16] = -15;  // 2000Hz
        eq.gain_db[17] = -15;  // 5000Hz
        eq.enabled = true;
        
    } else if (strcmp(preset, "Bookshelf") == 0) {
        // Compensate for small bookshelf speakers (light bass boost, bright highs)
        eq.gain_db[0] = 4;    // 40Hz
        eq.gain_db[1] = 4;    // 50Hz
        eq.gain_db[2] = 3;    // 60Hz
        eq.gain_db[3] = 3;    // 70Hz
        eq.gain_db[4] = 2;    // 80Hz
        eq.gain_db[5] = 2;    // 90Hz
        eq.gain_db[6] = 1;    // 100Hz
        eq.gain_db[7] = 1;    // 110Hz
        eq.gain_db[8] = 0;    // 120Hz
        eq.gain_db[9] = 0;    // 130Hz
        eq.gain_db[10] = 0;   // 140Hz
        eq.gain_db[11] = 0;   // 200Hz
        eq.gain_db[12] = -1;  // 315Hz
        eq.gain_db[13] = 0;   // 500Hz
        eq.gain_db[14] = 1;   // 800Hz
        eq.gain_db[15] = 2;   // 1250Hz
        eq.gain_db[16] = 2;   // 2000Hz
        eq.gain_db[17] = 3;   // 5000Hz
        eq.enabled = true;
        
    } else if (strcmp(preset, "Floor Standing") == 0) {
        // Balanced full-range curve for tower/floor speakers
        eq.gain_db[0] = 3;    // 40Hz
        eq.gain_db[1] = 2;    // 50Hz
        eq.gain_db[2] = 2;    // 60Hz
        eq.gain_db[3] = 1;    // 70Hz
        eq.gain_db[4] = 1;    // 80Hz
        eq.gain_db[5] = 1;    // 90Hz
        eq.gain_db[6] = 0;    // 100Hz
        eq.gain_db[7] = 0;    // 110Hz
        eq.gain_db[8] = 0;    // 120Hz
        eq.gain_db[9] = 0;    // 130Hz
        eq.gain_db[10] = -1;  // 140Hz
        eq.gain_db[11] = -1;  // 200Hz
        eq.gain_db[12] = -1;  // 315Hz
        eq.gain_db[13] = 0;   // 500Hz
        eq.gain_db[14] = 0;   // 800Hz
        eq.gain_db[15] = 1;   // 1250Hz
        eq.gain_db[16] = 1;   // 2000Hz
        eq.gain_db[17] = 2;   // 5000Hz
        eq.enabled = true;
        
    } else if (strcmp(preset, "Near Field") == 0) {
        // Studio monitor curve - neutral with slight presence boost
        eq.gain_db[0] = -1;   // 40Hz
        eq.gain_db[1] = -1;   // 50Hz
        eq.gain_db[2] = 0;    // 60Hz
        eq.gain_db[3] = 0;    // 70Hz
        eq.gain_db[4] = 0;    // 80Hz
        eq.gain_db[5] = 0;    // 90Hz
        eq.gain_db[6] = 0;    // 100Hz
        eq.gain_db[7] = 0;    // 110Hz
        eq.gain_db[8] = 0;    // 120Hz
        eq.gain_db[9] = 0;    // 130Hz
        eq.gain_db[10] = 0;   // 140Hz
        eq.gain_db[11] = 0;   // 200Hz
        eq.gain_db[12] = 0;   // 315Hz
        eq.gain_db[13] = 0;   // 500Hz
        eq.gain_db[14] = 0;   // 800Hz
        eq.gain_db[15] = 1;   // 1250Hz
        eq.gain_db[16] = 1;   // 2000Hz
        eq.gain_db[17] = 0;   // 5000Hz
        eq.enabled = true;
        
    } else if (strcmp(preset, "Small/Portable") == 0) {
        // For laptop/phone speakers - cut deep bass, boost mids/highs
        eq.gain_db[0] = -8;   // 40Hz
        eq.gain_db[1] = -6;   // 50Hz
        eq.gain_db[2] = -5;   // 60Hz
        eq.gain_db[3] = -4;   // 70Hz
        eq.gain_db[4] = -3;   // 80Hz
        eq.gain_db[5] = -2;   // 90Hz
        eq.gain_db[6] = -1;   // 100Hz
        eq.gain_db[7] = 0;    // 110Hz
        eq.gain_db[8] = 0;    // 120Hz
        eq.gain_db[9] = 1;    // 130Hz
        eq.gain_db[10] = 1;   // 140Hz
        eq.gain_db[11] = 2;   // 200Hz
        eq.gain_db[12] = 2;   // 315Hz
        eq.gain_db[13] = 2;   // 500Hz
        eq.gain_db[14] = 3;   // 800Hz
        eq.gain_db[15] = 4;   // 1250Hz
        eq.gain_db[16] = 4;   // 2000Hz
        eq.gain_db[17] = 5;   // 5000Hz
        eq.enabled = true;
        
    } else if (strcmp(preset, "Custom") == 0) {
        // Keep current values
        eq.enabled = true;
        return;
    }
    
    // Regenerate all coefficients
    eq_init(eq.sample_rate);
}

/**
 * ESPHome callback: Enable/disable entire EQ
 */
extern "C" inline void enable_eq(bool enable) {
    eq.enabled = enable;
    ESP_LOGI(EQ_TAG, "EQ %s", enable ? "enabled" : "disabled");
    
    if (enable) {
        // Clear delay lines to avoid clicks
        memset(eq.delay_left, 0, sizeof(eq.delay_left));
        memset(eq.delay_right, 0, sizeof(eq.delay_right));
    }
}

#endif // GRAPHIC_EQ_H
