

#ifndef SBR_GET_SCE_H
#define SBR_GET_SCE_H

#include    "pv_audio_type_defs.h"
#include    "s_bit_buffer.h"
#include    "s_sbr_frame_data.h"
#include    "e_sbr_error.h"
#include    "config.h"
#ifdef PARAMETRICSTEREO
#include    "s_ps_dec.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    SBR_ERROR sbr_get_sce(SBR_FRAME_DATA * hFrameData,
    BIT_BUFFER * hBitBuf
#ifdef PARAMETRICSTEREO
    , HANDLE_PS_DEC hParametricStereoDec
#endif
                         );

#ifdef __cplusplus
}
#endif

#endif

