

#ifndef OPUS_PROJECTION_H
#define OPUS_PROJECTION_H

#include "opus_multistream.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OPUS_PROJECTION_GET_DEMIXING_MATRIX_GAIN_REQUEST    6001
#define OPUS_PROJECTION_GET_DEMIXING_MATRIX_SIZE_REQUEST    6003
#define OPUS_PROJECTION_GET_DEMIXING_MATRIX_REQUEST         6005

#define OPUS_PROJECTION_GET_DEMIXING_MATRIX_GAIN(x) OPUS_PROJECTION_GET_DEMIXING_MATRIX_GAIN_REQUEST, __opus_check_int_ptr(x)

#define OPUS_PROJECTION_GET_DEMIXING_MATRIX_SIZE(x) OPUS_PROJECTION_GET_DEMIXING_MATRIX_SIZE_REQUEST, __opus_check_int_ptr(x)

#define OPUS_PROJECTION_GET_DEMIXING_MATRIX(x,y) OPUS_PROJECTION_GET_DEMIXING_MATRIX_REQUEST, x, __opus_check_int(y)

typedef struct OpusProjectionEncoder OpusProjectionEncoder;

typedef struct OpusProjectionDecoder OpusProjectionDecoder;

OPUS_EXPORT OPUS_WARN_UNUSED_RESULT opus_int32 opus_projection_ambisonics_encoder_get_size(
    int channels,
    int mapping_family
);

OPUS_EXPORT OPUS_WARN_UNUSED_RESULT OpusProjectionEncoder *opus_projection_ambisonics_encoder_create(
    opus_int32 Fs,
    int channels,
    int mapping_family,
    int *streams,
    int *coupled_streams,
    int application,
    int *error
) OPUS_ARG_NONNULL(4) OPUS_ARG_NONNULL(5);

OPUS_EXPORT int opus_projection_ambisonics_encoder_init(
    OpusProjectionEncoder *st,
    opus_int32 Fs,
    int channels,
    int mapping_family,
    int *streams,
    int *coupled_streams,
    int application
) OPUS_ARG_NONNULL(1) OPUS_ARG_NONNULL(5) OPUS_ARG_NONNULL(6);

OPUS_EXPORT OPUS_WARN_UNUSED_RESULT int opus_projection_encode(
    OpusProjectionEncoder *st,
    const opus_int16 *pcm,
    int frame_size,
    unsigned char *data,
    opus_int32 max_data_bytes
) OPUS_ARG_NONNULL(1) OPUS_ARG_NONNULL(2) OPUS_ARG_NONNULL(4);

OPUS_EXPORT OPUS_WARN_UNUSED_RESULT int opus_projection_encode_float(
    OpusProjectionEncoder *st,
    const float *pcm,
    int frame_size,
    unsigned char *data,
    opus_int32 max_data_bytes
) OPUS_ARG_NONNULL(1) OPUS_ARG_NONNULL(2) OPUS_ARG_NONNULL(4);

OPUS_EXPORT void opus_projection_encoder_destroy(OpusProjectionEncoder *st);

OPUS_EXPORT int opus_projection_encoder_ctl(OpusProjectionEncoder *st, int request, ...) OPUS_ARG_NONNULL(1);

OPUS_EXPORT OPUS_WARN_UNUSED_RESULT opus_int32 opus_projection_decoder_get_size(
    int channels,
    int streams,
    int coupled_streams
);

OPUS_EXPORT OPUS_WARN_UNUSED_RESULT OpusProjectionDecoder *opus_projection_decoder_create(
    opus_int32 Fs,
    int channels,
    int streams,
    int coupled_streams,
    unsigned char *demixing_matrix,
    opus_int32 demixing_matrix_size,
    int *error
) OPUS_ARG_NONNULL(5);

OPUS_EXPORT int opus_projection_decoder_init(
    OpusProjectionDecoder *st,
    opus_int32 Fs,
    int channels,
    int streams,
    int coupled_streams,
    unsigned char *demixing_matrix,
    opus_int32 demixing_matrix_size
) OPUS_ARG_NONNULL(1) OPUS_ARG_NONNULL(6);

OPUS_EXPORT OPUS_WARN_UNUSED_RESULT int opus_projection_decode(
    OpusProjectionDecoder *st,
    const unsigned char *data,
    opus_int32 len,
    opus_int16 *pcm,
    int frame_size,
    int decode_fec
) OPUS_ARG_NONNULL(1) OPUS_ARG_NONNULL(4);

OPUS_EXPORT OPUS_WARN_UNUSED_RESULT int opus_projection_decode_float(
    OpusProjectionDecoder *st,
    const unsigned char *data,
    opus_int32 len,
    float *pcm,
    int frame_size,
    int decode_fec
) OPUS_ARG_NONNULL(1) OPUS_ARG_NONNULL(4);

OPUS_EXPORT int opus_projection_decoder_ctl(OpusProjectionDecoder *st, int request, ...) OPUS_ARG_NONNULL(1);

OPUS_EXPORT void opus_projection_decoder_destroy(OpusProjectionDecoder *st);

#ifdef __cplusplus
}
#endif

#endif
