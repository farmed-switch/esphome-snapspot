

#ifndef SBR_GET_ENVELOPE_H
#define SBR_GET_ENVELOPE_H

#include "pv_audio_type_defs.h"
#include    "s_bit_buffer.h"
#include    "s_sbr_frame_data.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void sbr_get_envelope(SBR_FRAME_DATA * h_frame_data,
    BIT_BUFFER  * hBitBuf);

#ifdef __cplusplus
}
#endif

#endif

