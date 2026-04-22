#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_log.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define EQ_BANDS 18

#define EQ_TAG "GraphicEQ"

typedef struct {
    float    center_freq[EQ_BANDS];
    float    q_factor[EQ_BANDS];
    float    gain_db[EQ_BANDS];
    float    coeffs[EQ_BANDS][5];

    bool     enabled;
    uint32_t sample_rate;
    int      band_count;

#define EQ_MAX_SOURCES 4
    float    source_gain[EQ_MAX_SOURCES];
} graphic_eq_t;
