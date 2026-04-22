

#ifndef SBR_EXTRACT_EXTENDED_DATA_H
#define SBR_EXTRACT_EXTENDED_DATA_H

#include    "pv_audio_type_defs.h"
#include    "s_bit_buffer.h"
#include    "s_sbr_frame_data.h"
#include    "config.h"
#ifdef PARAMETRICSTEREO
#include    "s_ps_dec.h"
#endif

void sbr_extract_extended_data(BIT_BUFFER * hBitBuf
#ifdef PARAMETRICSTEREO
                               , HANDLE_PS_DEC hParametricStereoDec
#endif
                              );

#endif

