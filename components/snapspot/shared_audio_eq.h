#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void eq_init(uint32_t sample_rate);

void eq_set_sample_rate(uint32_t sample_rate);

void eq_set_mode(int bands);

void eq_process_stereo_int16(int16_t *samples, size_t num_samples);

void set_eq_band(int band, float gain_db);

void apply_eq_preset(const char *preset);

void enable_eq(bool enable);

typedef enum {
    EQ_SOURCE_CSPOT      = 0,
    EQ_SOURCE_SNAPCLIENT = 1,
    EQ_SOURCE_MAX        = 4
} eq_source_t;

void eq_set_source_gain(eq_source_t source, float gain);

float eq_get_source_gain(eq_source_t source);

void eq_process_with_volume(int16_t *samples, size_t num_frames, eq_source_t source);

void hw_dsp_set_band(int band, float gain_db);

#ifdef __cplusplus
}
#endif
