#include "graphic_eq.h"
#include "shared_audio_eq.h"

#include <math.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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
    .source_gain = {1.0f, 1.0f, 1.0f, 1.0f},
};

static portMUX_TYPE eq_spinlock = portMUX_INITIALIZER_UNLOCKED;

extern "C" void hw_dsp_set_band(int band, float gain_db);

#define EQ_FADE_FRAMES 882
static volatile int  eq_fade_in_frames  = 0;
static volatile int  eq_fade_out_frames = 0;

static volatile bool eq_delay_reset_pending = false;

static void compute_band_coeffs(float *out, float norm_freq, float q, float gain_db)
{
    if (gain_db == 0.0f) {
        out[0] = 1.0f; out[1] = 0.0f; out[2] = 0.0f;
        out[3] = 0.0f; out[4] = 0.0f;
        return;
    }

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

void eq_init(uint32_t sample_rate)
{

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

    float temp_coeffs[EQ_BANDS][5];
    for (int band = 0; band < band_count; band++) {
        float norm_freq = center_freq[band] / (float)sample_rate;
        compute_band_coeffs(temp_coeffs[band], norm_freq, q_factor[band], gain_db[band]);
        ESP_LOGD(EQ_TAG, "Band %d: %.0f Hz, Q=%.1f, Gain=%.1f dB",
                 band, center_freq[band], q_factor[band], gain_db[band]);
    }

    taskENTER_CRITICAL(&eq_spinlock);
    eq.sample_rate = sample_rate;
    memcpy(eq.coeffs, temp_coeffs, band_count * 5 * sizeof(float));
    taskEXIT_CRITICAL(&eq_spinlock);
    eq_delay_reset_pending = true;
}

void eq_set_sample_rate(uint32_t sample_rate)
{
    uint32_t old_sr;
    taskENTER_CRITICAL(&eq_spinlock);
    old_sr = eq.sample_rate;
    taskEXIT_CRITICAL(&eq_spinlock);

    if (old_sr != sample_rate) {
        ESP_LOGI(EQ_TAG, "Sample rate changed: %lu -> %lu Hz",
                 (unsigned long)old_sr, (unsigned long)sample_rate);
        eq_init(sample_rate);
    }
}

void eq_process_stereo_int16(int16_t *samples, size_t num_samples)
{

    static float delay_left [EQ_BANDS][2] = {};
    static float delay_right[EQ_BANDS][2] = {};

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

    if (eq_delay_reset_pending) {
        memset(delay_left,  0, sizeof(delay_left));
        memset(delay_right, 0, sizeof(delay_right));
        eq_delay_reset_pending = false;
        ESP_LOGD(EQ_TAG, "eq_process: delay lines reset");
    }

    int fade_in  = eq_fade_in_frames;
    int fade_out = eq_fade_out_frames;

    if (!enabled && fade_in == 0 && fade_out == 0) return;

    const float threshold = 0.7f;

    for (size_t i = 0; i < num_samples; i++) {
        float dry_l = samples[i * 2]     / 32768.0f;
        float dry_r = samples[i * 2 + 1] / 32768.0f;
        float left  = dry_l;
        float right = dry_r;

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

        float mix = 1.0f;
        if (fade_in > 0) {
            mix = 1.0f - (float)fade_in / (float)EQ_FADE_FRAMES;
            fade_in--;
        } else if (fade_out > 0) {
            mix = (float)fade_out / (float)EQ_FADE_FRAMES;
            fade_out--;
        }

        left  = dry_l + (left  - dry_l) * mix;
        right = dry_r + (right - dry_r) * mix;

        samples[i * 2]     = (int16_t)(left  * 32767.0f);
        samples[i * 2 + 1] = (int16_t)(right * 32767.0f);
    }

    eq_fade_in_frames  = fade_in;
    eq_fade_out_frames = fade_out;

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

    float new_coeffs[5];
    float norm_freq = center / (float)sr;
    compute_band_coeffs(new_coeffs, norm_freq, q, gain_db);

    taskENTER_CRITICAL(&eq_spinlock);
    memcpy(eq.coeffs[band], new_coeffs, 5 * sizeof(float));
    taskEXIT_CRITICAL(&eq_spinlock);

    hw_dsp_set_band(band, gain_db);
}

void eq_set_source_gain(eq_source_t source, float gain)
{
    if ((int)source < 0 || (int)source >= EQ_MAX_SOURCES) return;
    if (gain < 0.0f) gain = 0.0f;
    if (gain > 1.0f) gain = 1.0f;

    taskENTER_CRITICAL(&eq_spinlock);
    eq.source_gain[(int)source] = gain;
    taskEXIT_CRITICAL(&eq_spinlock);

    ESP_LOGD(EQ_TAG, "Source %d gain set to %.3f", (int)source, gain);
}

float eq_get_source_gain(eq_source_t source)
{
    if ((int)source < 0 || (int)source >= EQ_MAX_SOURCES) return 0.0f;

    float g;
    taskENTER_CRITICAL(&eq_spinlock);
    g = eq.source_gain[(int)source];
    taskEXIT_CRITICAL(&eq_spinlock);
    return g;
}

void eq_process_with_volume(int16_t *samples, size_t num_frames, eq_source_t source)
{
    if ((int)source < 0 || (int)source >= EQ_MAX_SOURCES) return;

    static float dl_left [EQ_MAX_SOURCES][EQ_BANDS][2] = {};
    static float dl_right[EQ_MAX_SOURCES][EQ_BANDS][2] = {};

    bool  enabled;
    int   band_count;
    float coeffs[EQ_BANDS][5];
    float src_gain;

    taskENTER_CRITICAL(&eq_spinlock);
    enabled    = eq.enabled;
    band_count = eq.band_count;
    src_gain   = eq.source_gain[(int)source];
    if (enabled) {
        memcpy(coeffs, eq.coeffs, band_count * 5 * sizeof(float));
    }
    taskEXIT_CRITICAL(&eq_spinlock);

    if (eq_delay_reset_pending) {
        memset(dl_left,  0, sizeof(dl_left));
        memset(dl_right, 0, sizeof(dl_right));
        eq_delay_reset_pending = false;
        ESP_LOGD(EQ_TAG, "eq_process_with_volume: delay lines reset");
    }

    int fade_in  = eq_fade_in_frames;
    int fade_out = eq_fade_out_frames;

    bool need_eq   = enabled || fade_in > 0 || fade_out > 0;
    bool need_gain = (src_gain < 0.999f);

    if (!need_eq && !need_gain) return;

    const int src = (int)source;
    const float threshold = 0.7f;

    for (size_t i = 0; i < num_frames; i++) {
        float left  = samples[i * 2]     / 32768.0f;
        float right = samples[i * 2 + 1] / 32768.0f;

        if (need_gain) {
            left  *= src_gain;
            right *= src_gain;
        }

        if (need_eq) {
            float dry_l = left;
            float dry_r = right;

            for (int band = 0; band < band_count; band++) {
                float w_left = left
                    - coeffs[band][3] * dl_left[src][band][0]
                    - coeffs[band][4] * dl_left[src][band][1];
                left = coeffs[band][0] * w_left
                     + coeffs[band][1] * dl_left[src][band][0]
                     + coeffs[band][2] * dl_left[src][band][1];
                dl_left[src][band][1] = dl_left[src][band][0];
                dl_left[src][band][0] = w_left;

                float w_right = right
                    - coeffs[band][3] * dl_right[src][band][0]
                    - coeffs[band][4] * dl_right[src][band][1];
                right = coeffs[band][0] * w_right
                      + coeffs[band][1] * dl_right[src][band][0]
                      + coeffs[band][2] * dl_right[src][band][1];
                dl_right[src][band][1] = dl_right[src][band][0];
                dl_right[src][band][0] = w_right;
            }

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

            float mix = 1.0f;
            if (fade_in > 0) {
                mix = 1.0f - (float)fade_in / (float)EQ_FADE_FRAMES;
                fade_in--;
            } else if (fade_out > 0) {
                mix = (float)fade_out / (float)EQ_FADE_FRAMES;
                fade_out--;
            }
            left  = dry_l + (left  - dry_l) * mix;
            right = dry_r + (right - dry_r) * mix;
        }

        samples[i * 2]     = (int16_t)(left  * 32767.0f);
        samples[i * 2 + 1] = (int16_t)(right * 32767.0f);
    }

    if (need_eq) {
        eq_fade_in_frames  = fade_in;
        eq_fade_out_frames = fade_out;
    }
}

extern "C" __attribute__((weak)) void hw_dsp_set_band(int band, float gain_db)
{

    (void)band; (void)gain_db;
}

void apply_eq_preset(const char *preset)
{
    ESP_LOGI(EQ_TAG, "Applying preset: %s", preset);

    if (strcmp(preset, "Custom") == 0) {
        taskENTER_CRITICAL(&eq_spinlock);
        eq.enabled = true;
        taskEXIT_CRITICAL(&eq_spinlock);
        return;
    }

    int bc;
    taskENTER_CRITICAL(&eq_spinlock);
    bc = eq.band_count;
    taskEXIT_CRITICAL(&eq_spinlock);

    float gains[EQ_BANDS] = {0};
    bool  do_enable;

    if (strcmp(preset, "Flat") == 0) {
        do_enable = false;

    } else if (strcmp(preset, "Bass Boost") == 0) {
        if (bc == 15) {
            float g[] = {10,8,6,3,-2,-8,-13,-15,-15,-15,-15,-15,-15,-15,-15};
            memcpy(gains, g, sizeof(g));
        } else {
            float g[] = {10,9,8,7,5,3,1,-2,-8,-13,-15,-15,-15,-15,-15,-15,-15,-15};
            memcpy(gains, g, sizeof(g));
        }
        do_enable = true;

    } else if (strcmp(preset, "Treble Boost") == 0) {
        if (bc == 15) {
            float g[] = {0,0,0,0,0,0,0,0,0,1,2,3,4,5,4};
            memcpy(gains, g, sizeof(g));
        } else {
            float g[] = {0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,5,4};
            memcpy(gains, g, sizeof(g));
        }
        do_enable = true;

    } else if (strcmp(preset, "Loudness") == 0) {
        if (bc == 15) {
            float g[] = {5,4,3,1,0,-1,-2,-1,0,0,1,2,3,4,3};
            memcpy(gains, g, sizeof(g));
        } else {
            float g[] = {5,4,4,3,2,1,0,0,-1,-2,-1,0,0,1,2,3,4,3};
            memcpy(gains, g, sizeof(g));
        }
        do_enable = true;

    } else if (strcmp(preset, "Vocal Clarity") == 0) {
        if (bc == 15) {
            float g[] = {0,0,0,0,0,-1,0,1,2,3,2,1,0,0,0};
            memcpy(gains, g, sizeof(g));
        } else {
            float g[] = {0,0,0,0,0,0,0,0,-1,0,0,1,1,2,3,2,1,0};
            memcpy(gains, g, sizeof(g));
        }
        do_enable = true;

    } else if (strcmp(preset, "Small Speaker") == 0) {
        if (bc == 15) {
            float g[] = {-6,-4,-2,0,1,1,2,2,2,2,3,3,4,3,2};
            memcpy(gains, g, sizeof(g));
        } else {
            float g[] = {-6,-5,-4,-3,-2,-1,0,1,1,1,2,2,2,3,3,4,3,2};
            memcpy(gains, g, sizeof(g));
        }
        do_enable = true;

    } else if (strcmp(preset, "Night Mode") == 0) {
        if (bc == 15) {
            float g[] = {-4,-3,-2,-1,0,0,1,1,1,0,0,-1,-2,-3,-4};
            memcpy(gains, g, sizeof(g));
        } else {
            float g[] = {-4,-3,-3,-2,-1,-1,0,0,0,1,1,0,0,-1,-2,-3,-4,-4};
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

        eq_delay_reset_pending = true;
        eq.enabled         = true;
        eq_fade_in_frames  = EQ_FADE_FRAMES;
        eq_fade_out_frames = 0;
    } else if (!enable && eq.enabled) {

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

        taskEXIT_CRITICAL(&eq_spinlock);
    } else {

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

        taskEXIT_CRITICAL(&eq_spinlock);
    }

    eq_init(sr);
}
