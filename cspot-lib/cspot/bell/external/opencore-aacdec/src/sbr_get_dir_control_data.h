

#ifndef SBR_GET_DIR_CONTROL_DATA_H
#define SBR_GET_DIR_CONTROL_DATA_H

#include "pv_audio_type_defs.h"
#include    "s_bit_buffer.h"
#include    "s_sbr_frame_data.h"

void sbr_get_dir_control_data(SBR_FRAME_DATA * h_frame_data,
                              BIT_BUFFER * hBitBuf);

#endif

