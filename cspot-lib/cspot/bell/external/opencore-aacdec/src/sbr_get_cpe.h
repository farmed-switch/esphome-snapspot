

#ifndef SBR_GET_CPE_H
#define SBR_GET_CPE_H

#include    "pv_audio_type_defs.h"
#include    "s_bit_buffer.h"
#include    "s_sbr_frame_data.h"
#include    "e_sbr_error.h"

SBR_ERROR sbr_get_cpe(SBR_FRAME_DATA * hFrameDataLeft,
                      SBR_FRAME_DATA * hFrameDataRight,
                      BIT_BUFFER * hBitBuf);

#endif

