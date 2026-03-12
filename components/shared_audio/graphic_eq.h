#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_log.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/** Number of EQ bands: 11 bass (40-140 Hz) + 7 higher (200-5000 Hz). */
#define EQ_BANDS 18

#define EQ_TAG "GraphicEQ"

/**
 * 18-band graphic equalizer state.
 *
 * center_freq[] and q_factor[] are fixed at startup.
 * gain_db[] is set by set_eq_band() / apply_eq_preset().
 * coeffs[], delay_left[], delay_right[] are maintained internally.
 */
typedef struct {
    float    center_freq[EQ_BANDS];     /* Hz: 40,50,...,140, 200,315,500,800,1250,2000,5000 */
    float    q_factor[EQ_BANDS];        /* bandwidth (1.0 = one-octave) */
    float    gain_db[EQ_BANDS];         /* ±15 dB per band */
    float    coeffs[EQ_BANDS][5];       /* biquad: b0,b1,b2,a1,a2 (Direct Form II Transposed) */
    /* NOTE: delay lines (w[n-1], w[n-2]) are intentionally NOT stored here.
     * They are owned exclusively by the audio task (eq_process_stereo_int16)
     * as static local arrays. Keeping them here would create a data race:
     * eq_init() clears them under spinlock while eq_process() writes them
     * without spinlock — causing NaN/silence on mode switches. */
    bool     enabled;                   /* master bypass flag */
    uint32_t sample_rate;               /* Hz, e.g. 44100 */
    int      band_count;                /* active bands: 15 (ISO) or 18 (default). Max = EQ_BANDS */
} graphic_eq_t;
