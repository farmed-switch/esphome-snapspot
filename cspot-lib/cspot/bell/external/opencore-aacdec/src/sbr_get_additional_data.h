

#ifndef SBR_GET_ADDITIONAL_DATA_H
#define SBR_GET_ADDITIONAL_DATA_H

#include "pv_audio_type_defs.h"
#include    "s_bit_buffer.h"
#include    "s_sbr_header_data.h"
#include    "s_sbr_frame_data.h"

void sbr_get_additional_data(SBR_FRAME_DATA * hFrameData,
                             BIT_BUFFER     * hBitBuf);

#endif

