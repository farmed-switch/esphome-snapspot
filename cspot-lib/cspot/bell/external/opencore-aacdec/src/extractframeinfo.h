

#ifndef EXTRACTFRAMEINFO_H
#define EXTRACTFRAMEINFO_H

#include    "pv_audio_type_defs.h"
#include    "s_bit_buffer.h"
#include    "s_sbr_frame_data.h"
#include    "e_sbr_error.h"

typedef Int32 FRAME_INFO[LENGTH_FRAME_INFO];

#ifdef __cplusplus
extern "C"
{
#endif

    SBR_ERROR extractFrameInfo(BIT_BUFFER     * hBitBuf,
    SBR_FRAME_DATA * h_frame_data);

#ifdef __cplusplus
}
#endif

#endif

