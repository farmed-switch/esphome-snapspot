#include "graphic_eq.h"
#include "shared_audio_eq.h"

#include <math.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * Global EQ instance.
 *
 * center_freq[] and q_factor[] are fixed constants matching the 18-band layout
 * described in GRAPHIC_EQ_README.md. gain_db[] defaults to 0 (flat/bypass).
 */
graphic_eq_t eq = {
    .center_freq = {
        40.f, 50.f, 60.f, 70.f, 80.f, 90.f,
        100.f, 110.f, 120.f, 130.f, 140.f,
        200.f, 315.f, 500.f, 800.f, 1250.f, 2000.f, 5000.f
    },
    .q_factor = {
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
    },
    .gain_db     = {0},
    .coeffs      = {{{0}}},
    .enabled     = false,
    .sample_rate = 44100,
    .band_count  = 18,
};

/**
 * Spinlock protecting .coeffs[], .enabled, .gain_db[], .band_count, and
 * .sample_rate from concurrent access between the audio task and the ESPHome
 * main-loop callbacks. Delay lines are NOT protected here — they are owned
 * exclusively by eq_process_stereo_int16() as static local arrays.
 *
 * Critical sections are kept short:
 *   - Audio path:   snapshot of .enabled + memcpy of coeffs (~360 B).
 *   - Update path:  memcpy of gains + scalar writes.
 * Delay lines are only written by eq_process_stereo_int16 and need no extra
 * protection as long as that function runs single-threaded.
 */
static portMUX_TYPE eq_spinlock = portMUX_INITIALIZER_UNLOCKED;

/* Forward declaration — weak definition is at the bottom of this file. */
extern "C" void hw_dsp_set_band(int band, float gain_db);

/* Fade-in/out counter — set to FADE_FRAMES on enable, decremented in
 * eq_process_stereo_int16. Avoids audible click when EQ is toggled.
 * FADE_FRAMES = 44100 * 20ms / 1000 ≈ 882 frames. */
#define EQ_FADE_FRAMES 882
static volatile int  eq_fade_in_frames  = 0;   /* >0 → ramping up   */
static volatile int  eq_fade_out_frames = 0;   /* >0 → ramping down */

/* Delay-line reset flag — set by eq_init() (main task), consumed by
 * eq_process_stereo_int16() (audio task). The audio task owns the delay
 * lines and is the only writer; consuming the flag here avoids any data
 * race that would occur if eq_init() cleared delay_left/right directly
 * while eq_process() was mid-way through writing them. */
static volatile bool eq_delay_reset_pending = false;

/* -------------------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------------- */

/** Compute biquad coefficients for one band into out[5].
 *  Uses the peaking-EQ formula from the Audio EQ Cookbook (Bristow-Johnson),
 *  adapted to match the c-MM-snapclient reference implementation exactly.
 *  gain_db == 0 produces unity-gain bypass coefficients.
 */
static void compute_band_coeffs(float *out, float norm_freq, float q, float gain_db)
{
    if (gain_db == 0.0f) {
        out[0] = 1.0f; out[1] = 0.0f; out[2] = 0.0f;
        out[3] = 0.0f; out[4] = 0.0f;
        return;
    }

    /* A = 10^(dBgain/40) per Audio EQ Cookbook (Bristow-Johnson).
     * This gives |H(jω0)| = A² = 10^(dBgain/20), i.e. the actual gain at
     * the centre frequency equals exactly gain_db dB.
     * Using /20 instead of /40 would double the effective dB gain. */
    float A           = powf(10.0f, gain_db / 40.0f);
    float w0          = 2.0f * (float)M_PI * norm_freq;
    float alpha       = sinf(w0) / (2.0f * q);

    float b0 = 1.0f + alpha * A;
    float b1 = -2.0f * cosf(w0);
    float b2 = 1.0f - alpha * A;
    float a0 = 1.0f + alpha / A;
    float a1 = -2.0f * cosf(w0);
    float a2 = 1.0f - alpha / A;

    out[0] = b0 / a0;
    out[1] = b1 / a0;
    out[2] = b2 / a0;
    out[3] = a1 / a0;
    out[4] = a2 / a0;
}

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

void eq_init(uint32_t sample_rate)
{
    /* Snapshot input state without holding the lock during expensive math. */
    int band_count;
    float center_freq[EQ_BANDS];
    float q_factor[EQ_BANDS];
    float gain_db[EQ_BANDS];

    taskENTER_CRITICAL(&eq_spinlock);
    band_count = eq.band_count;
    memcpy(center_freq, eq.center_freq, sizeof(center_freq));
    memcpy(q_factor,    eq.q_factor,    sizeof(q_factor));
    memcpy(gain_db,     eq.gain_db,     sizeof(gain_db));
    taskEXIT_CRITICAL(&eq_spinlock);

    ESP_LOGI(EQ_TAG, "Initializing %d-band graphic EQ @ %lu Hz",
             band_count, (unsigned long)sample_rate);

    /* Compute into local buffer — no lock held during trig operations.
     * norm_freq = f/fs so that w0 = 2π·f/fs (Audio EQ Cookbook standard).
     * Using f/(fs/2) would double w0, making high-freq bands unstable (poles
     * outside the unit circle → delay-line runaway → silence + clicks). */
    float temp_coeffs[EQ_BANDS][5];
    for (int band = 0; band < band_count; band++) {
        float norm_freq = center_freq[band] / (float)sample_rate;
        compute_band_coeffs(temp_coeffs[band], norm_freq, q_factor[band], gain_db[band]);
        ESP_LOGD(EQ_TAG, "Band %d: %.0f Hz, Q=%.1f, Gain=%.1f dB",
                 band, center_freq[band], q_factor[band], gain_db[band]);
    }

    /* Atomically install new coefficients, then signal the audio task to
     * reset its delay lines. We must NOT write delay_left/right here because
     * eq_process_stereo_int16 may be running concurrently on the audio task
     * and writes those arrays without holding the spinlock (inner-loop perf).
     * Setting the flag is safe: it is written once here and read/cleared once
     * in eq_process at the top of the next buffer. */
    taskENTER_CRITICAL(&eq_spinlock);
    eq.sample_rate = sample_rate;
    memcpy(eq.coeffs, temp_coeffs, band_count * 5 * sizeof(float));
    taskEXIT_CRITICAL(&eq_spinlock);
    eq_delay_reset_pending = true;
}

void eq_process_stereo_int16(int16_t *samples, size_t num_samples)
{
    /* Delay lines: private to the audio task. Static so they persist across
     * calls without allocation cost; zeroed at startup and whenever
     * eq_delay_reset_pending is set (by eq_init / enable_eq). */
    static float delay_left [EQ_BANDS][2] = {};
    static float delay_right[EQ_BANDS][2] = {};

    /* Snapshot enabled flag and coefficients under a brief critical section.
     * Delay lines are owned exclusively by this function — no lock needed. */
    bool  enabled;
    int   band_count;
    float coeffs[EQ_BANDS][5];

    taskENTER_CRITICAL(&eq_spinlock);
    enabled    = eq.enabled;
    band_count = eq.band_count;
    if (enabled) {
        memcpy(coeffs, eq.coeffs, band_count * 5 * sizeof(float));
    }
    taskEXIT_CRITICAL(&eq_spinlock);

    /* Consume pending delay-line reset requested by eq_init() or enable_eq().
     * This must run AFTER the spinlock read so we see the fresh coefficients
     * in the same buffer as the cleared state (no half-old-half-new frame). */
    if (eq_delay_reset_pending) {
        memset(delay_left,  0, sizeof(delay_left));
        memset(delay_right, 0, sizeof(delay_right));
        eq_delay_reset_pending = false;
        ESP_LOGD(EQ_TAG, "eq_process: delay lines reset");
    }

    /* Snapshot fade counter (only fade-in remains; fade-out is now always 0
     * after enable_eq(false) since we skip the audio-driven ramp). */
    int fade_in  = eq_fade_in_frames;
    int fade_out = eq_fade_out_frames;

    if (!enabled && fade_in == 0 && fade_out == 0) return;

    const float threshold = 0.7f;

    for (size_t i = 0; i < num_samples; i++) {
        float dry_l = samples[i * 2]     / 32768.0f;
        float dry_r = samples[i * 2 + 1] / 32768.0f;
        float left  = dry_l;
        float right = dry_r;

        /* Cascade active biquad filters (Direct Form II Transposed). */
        for (int band = 0; band < band_count; band++) {
            float w_left = left
                - coeffs[band][3] * delay_left[band][0]
                - coeffs[band][4] * delay_left[band][1];
            left = coeffs[band][0] * w_left
                 + coeffs[band][1] * delay_left[band][0]
                 + coeffs[band][2] * delay_left[band][1];
            delay_left[band][1] = delay_left[band][0];
            delay_left[band][0] = w_left;

            float w_right = right
                - coeffs[band][3] * delay_right[band][0]
                - coeffs[band][4] * delay_right[band][1];
            right = coeffs[band][0] * w_right
                  + coeffs[band][1] * delay_right[band][0]
                  + coeffs[band][2] * delay_right[band][1];
            delay_right[band][1] = delay_right[band][0];
            delay_right[band][0] = w_right;
        }

        /* Cubic soft-clipping at ±70% — prevents hard clipping on large gains.
         * Technique from the c-MM-snapclient reference (Andy's cubic soft-clip). */
        if (left > 1.0f) {
            left = 1.0f;
        } else if (left > threshold) {
            float x = (left - threshold) / (1.0f - threshold);
            left = threshold + (1.0f - threshold) * (x - x * x * x / 3.0f);
        } else if (left < -1.0f) {
            left = -1.0f;
        } else if (left < -threshold) {
            float x = (-left - threshold) / (1.0f - threshold);
            left = -threshold - (1.0f - threshold) * (x - x * x * x / 3.0f);
        }

        if (right > 1.0f) {
            right = 1.0f;
        } else if (right > threshold) {
            float x = (right - threshold) / (1.0f - threshold);
            right = threshold + (1.0f - threshold) * (x - x * x * x / 3.0f);
        } else if (right < -1.0f) {
            right = -1.0f;
        } else if (right < -threshold) {
            float x = (-right - threshold) / (1.0f - threshold);
            right = -threshold - (1.0f - threshold) * (x - x * x * x / 3.0f);
        }

        /* Compute fade multiplier for this frame. */
        float mix = 1.0f;
        if (fade_in > 0) {
            mix = 1.0f - (float)fade_in / (float)EQ_FADE_FRAMES;
            fade_in--;
        } else if (fade_out > 0) {
            mix = (float)fade_out / (float)EQ_FADE_FRAMES;
            fade_out--;
        }

        /* Blend EQ output with dry signal using fade multiplier. */
        left  = dry_l + (left  - dry_l) * mix;
        right = dry_r + (right - dry_r) * mix;

        samples[i * 2]     = (int16_t)(left  * 32767.0f);
        samples[i * 2 + 1] = (int16_t)(right * 32767.0f);
    }

    /* Write back fade counters after processing the full buffer.
     * Writing inside the loop (at i==0 only) caused the counters to decrease
     * by 1 per buffer instead of by num_samples — making the fade 128x too slow. */
    eq_fade_in_frames  = fade_in;
    eq_fade_out_frames = fade_out;
    /* Note: eq.enabled is already false (set immediately in enable_eq(false)).
     * No need to clear it here when fade-out completes. */
}

void set_eq_band(int band, float gain_db)
{
    if (band < 0 || band >= EQ_BANDS) {
        ESP_LOGE(EQ_TAG, "Invalid band index: %d (max %d)", band, EQ_BANDS - 1);
        return;
    }
    if (gain_db < -15.0f || gain_db > 15.0f) {
        ESP_LOGE(EQ_TAG, "Invalid gain %.1f dB for band %d (range: \u00b115)", gain_db, band);
        return;
    }

    float center, q, sr;
    int bc;
    taskENTER_CRITICAL(&eq_spinlock);
    bc = eq.band_count;
    if (band >= bc) {
        taskEXIT_CRITICAL(&eq_spinlock);
        ESP_LOGW(EQ_TAG, "Band %d out of range for current mode (%d bands)", band, bc);
        return;
    }
    eq.gain_db[band] = gain_db;
    center = eq.center_freq[band];
    q      = eq.q_factor[band];
    sr     = eq.sample_rate;
    taskEXIT_CRITICAL(&eq_spinlock);

    ESP_LOGI(EQ_TAG, "Band %d (%.0f Hz) \u2192 %.1f dB", band, center, gain_db);

    /* Recompute ONLY this band's coefficient outside the spinlock (trig is expensive).
     * Do NOT call eq_init() here — that resets ALL delay lines → audible click on
     * every slider move. Updating a single coefficient with existing delay state
     * produces at most a brief sub-millisecond transient. */
    float new_coeffs[5];
    float norm_freq = center / (float)sr;
    compute_band_coeffs(new_coeffs, norm_freq, q, gain_db);

    /* Install new coefficients atomically. */
    taskENTER_CRITICAL(&eq_spinlock);
    memcpy(eq.coeffs[band], new_coeffs, 5 * sizeof(float));
    taskEXIT_CRITICAL(&eq_spinlock);

    /* Forward to hardware DSP if a device-specific override is provided.
     * Default implementation is a no-op (weak symbol below). */
    hw_dsp_set_band(band, gain_db);
}

/* -------------------------------------------------------------------------
 * Driver-agnostic hardware DSP hook — weak / overridable
 * -----------------------------------------------------------------------
 * Override hw_dsp_set_band() in your device-specific component to forward
 * slider values to the hardware DSP chip (TAS5805M, TAS58xx, etc.).
 * Because this is a weak symbol, the linker uses this no-op unless an
 * override is present anywhere in the build.
 *
 * Example override for a TAS58xx component (in a device .cpp file):
 *
 *   extern "C" void hw_dsp_set_band(int band, float gain_db) {
 *       // id(tas5805m_dac) is the ESPHome component reference
 *       id(tas5805m_dac).set_eq_gain(band, gain_db);  // component-specific API
 *   }
 * ---------------------------------------------------------------------- */
extern "C" __attribute__((weak)) void hw_dsp_set_band(int band, float gain_db)
{
    /* Default: no-op. Hardware DSP not connected. */
    (void)band; (void)gain_db;
}

void apply_eq_preset(const char *preset)
{
    ESP_LOGI(EQ_TAG, "Applying preset: %s", preset);

    /* "Custom" just enables EQ without touching gains. */
    if (strcmp(preset, "Custom") == 0) {
        taskENTER_CRITICAL(&eq_spinlock);
        eq.enabled = true;
        taskEXIT_CRITICAL(&eq_spinlock);
        return;
    }

    /* Read band_count to select the right gain set. */
    int bc;
    taskENTER_CRITICAL(&eq_spinlock);
    bc = eq.band_count;
    taskEXIT_CRITICAL(&eq_spinlock);

    /* Build gains array outside the spinlock (strcmp, float literals).
     * Zero-initialised so unused upper slots stay at 0. */
    float gains[EQ_BANDS] = {0};
    bool  do_enable;

    if (strcmp(preset, "Flat (Bypass)") == 0) {
        do_enable = false;  /* Full bypass — no recompute needed. */

    } else if (strcmp(preset, "Flat (Full Range)") == 0) {
        do_enable = true;

    } else if (strcmp(preset, "Subwoofer") == 0) {
        if (bc == 15) {
            /* 15-band ISO: deep bass boost, rolls off above 160 Hz */
            float g[] = {10,9,7,5,3,1,-2,-6,-10,-13,-14,-15,-15,-15,-15};
            memcpy(gains, g, sizeof(g));
        } else {
            /* 18-band custom: dense bass boost + steep rolloff above 140 Hz */
            float g[] = {8,7,6,5,4,3,2,1,0,-10,-12,-13,-14,-15,-15,-15,-15,-15};
            memcpy(gains, g, sizeof(g));
        }
        do_enable = true;

    } else if (strcmp(preset, "Bookshelf") == 0) {
        if (bc == 15) {
            float g[] = {3,4,4,3,2,1,0,0,0,0,0,0,1,1,2};
            memcpy(gains, g, sizeof(g));
        } else {
            float g[] = {4,4,3,3,2,2,1,1,0,0,0,0,-1,0,1,2,2,3};
            memcpy(gains, g, sizeof(g));
        }
        do_enable = true;

    } else if (strcmp(preset, "Floor Standing") == 0) {
        if (bc == 15) {
            float g[] = {2,2,2,1,1,0,0,0,0,0,-1,-1,0,0,1};
            memcpy(gains, g, sizeof(g));
        } else {
            float g[] = {3,2,2,1,1,1,0,0,0,0,-1,-1,-1,0,0,1,1,2};
            memcpy(gains, g, sizeof(g));
        }
        do_enable = true;

    } else if (strcmp(preset, "Near Field") == 0) {
        if (bc == 15) {
            float g[] = {-1,-1,0,0,0,0,0,0,0,0,0,0,0,1,0};
            memcpy(gains, g, sizeof(g));
        } else {
            float g[] = {-1,-1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0};
            memcpy(gains, g, sizeof(g));
        }
        do_enable = true;

    } else if (strcmp(preset, "Small/Portable") == 0) {
        if (bc == 15) {
            float g[] = {-10,-8,-6,-4,-2,0,1,2,2,2,3,3,4,4,5};
            memcpy(gains, g, sizeof(g));
        } else {
            float g[] = {-8,-6,-5,-4,-3,-2,-1,0,0,1,1,2,2,2,3,4,4,5};
            memcpy(gains, g, sizeof(g));
        }
        do_enable = true;

    } else {
        ESP_LOGW(EQ_TAG, "Unknown preset: %s", preset);
        return;
    }

    uint32_t sr;
    taskENTER_CRITICAL(&eq_spinlock);
    memcpy(eq.gain_db, gains, sizeof(gains));
    eq.enabled = do_enable;
    sr = eq.sample_rate;
    taskEXIT_CRITICAL(&eq_spinlock);

    if (do_enable) {
        eq_init(sr);
    }
}

void enable_eq(bool enable)
{
    taskENTER_CRITICAL(&eq_spinlock);
    if (enable && !eq.enabled) {
        /* Signal the audio task to clear delay lines before fade-in.
         * We cannot clear them here (data race with eq_process inner loop);
         * the audio task owns the delay arrays and will reset them at the
         * start of the next buffer when it sees eq_delay_reset_pending. */
        eq_delay_reset_pending = true;
        eq.enabled         = true;
        eq_fade_in_frames  = EQ_FADE_FRAMES;
        eq_fade_out_frames = 0;
    } else if (!enable && eq.enabled) {
        /* Set eq.enabled=false AND reset both fade counters immediately.
         * Setting fade_out=0 (not EQ_FADE_FRAMES) is intentional: we cannot
         * rely on audio flowing to decrement the counter. If the device is
         * silent when the mode switches (e.g. snapclient not yet connected),
         * a non-zero fade_out would survive until the next audio buffer and
         * cause unexpected EQ processing. Hard-cut is correct here because
         * the mode switch itself is not audio-driven. */
        eq.enabled         = false;
        eq_fade_out_frames = 0;
        eq_fade_in_frames  = 0;
    }
    taskEXIT_CRITICAL(&eq_spinlock);

    ESP_LOGI(EQ_TAG, "EQ %s", enable ? "enabling (fade-in)" : "disabling (fade-out)");
}

void eq_set_mode(int bands)
{
    if (bands != 15 && bands != 18) {
        ESP_LOGE(EQ_TAG, "eq_set_mode: unsupported band count %d (use 15 or 18)", bands);
        return;
    }
    ESP_LOGI(EQ_TAG, "Setting EQ to %d-band mode", bands);

    uint32_t sr;
    taskENTER_CRITICAL(&eq_spinlock);
    sr = eq.sample_rate;
    taskEXIT_CRITICAL(&eq_spinlock);

    if (bands == 15) {
        /* ISO 2/3-octave 15-band: frequencies matching TAS5805M hardware DSP bands */
        static const float iso15_freq[15] = {
            25.f, 40.f, 63.f, 100.f, 160.f, 250.f,
            400.f, 630.f, 1000.f, 1600.f, 2500.f,
            4000.f, 6300.f, 10000.f, 16000.f
        };
        static const float iso15_q[15] = {
            1.4f, 1.4f, 1.4f, 1.4f, 1.4f, 1.4f,
            1.4f, 1.4f, 1.4f, 1.4f, 1.4f,
            1.4f, 1.4f, 1.4f, 1.4f
        };
        taskENTER_CRITICAL(&eq_spinlock);
        eq.band_count = 15;
        memcpy(eq.center_freq, iso15_freq, 15 * sizeof(float));
        memcpy(eq.q_factor,    iso15_q,    15 * sizeof(float));
        /* Do NOT reset gain_db — sliders hold the user's values in NVS. */
        taskEXIT_CRITICAL(&eq_spinlock);
    } else {
        /* Default 18-band custom: dense bass bands below 200 Hz */
        static const float orig18_freq[18] = {
            40.f, 50.f, 60.f, 70.f, 80.f, 90.f,
            100.f, 110.f, 120.f, 130.f, 140.f,
            200.f, 315.f, 500.f, 800.f, 1250.f, 2000.f, 5000.f
        };
        static const float orig18_q[18] = {
            1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f
        };
        taskENTER_CRITICAL(&eq_spinlock);
        eq.band_count = 18;
        memcpy(eq.center_freq, orig18_freq, 18 * sizeof(float));
        memcpy(eq.q_factor,    orig18_q,    18 * sizeof(float));
        /* Do NOT reset gain_db — sliders hold the user's values in NVS. */
        taskEXIT_CRITICAL(&eq_spinlock);
    }

    /* Recompute biquad coefficients for the new band layout. */
    eq_init(sr);
}
