

#ifndef SBR_GET_NOISE_FLOOR_DATA_H
#define SBR_GET_NOISE_FLOOR_DATA_H

#include "pv_audio_type_defs.h"
#include    "s_bit_buffer.h"
#include    "s_sbr_frame_data.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void sbr_get_noise_floor_data(SBR_FRAME_DATA * h_frame_data,
    BIT_BUFFER * hBitBuf);

#ifdef __cplusplus
}
#endif

#endif

