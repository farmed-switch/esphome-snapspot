

#ifndef SBR_GET_HEADER_DATA_H
#define SBR_GET_HEADER_DATA_H

#include "pv_audio_type_defs.h"
#include    "s_bit_buffer.h"
#include    "s_sbr_header_data.h"
#include    "e_sbr_element_id.h"
#include    "e_sbr_sync_state.h"

SBR_HEADER_STATUS sbr_get_header_data(SBR_HEADER_DATA   *h_sbr_header,
                                      BIT_BUFFER  * hBitBuf,
                                      SBR_SYNC_STATE     syncState);

#endif

