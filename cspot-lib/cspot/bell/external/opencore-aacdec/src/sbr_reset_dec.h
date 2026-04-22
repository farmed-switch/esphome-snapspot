

#ifndef SBR_RESET_DEC_H
#define SBR_RESET_DEC_H

#include    "s_sbr_frame_data.h"
#include    "sbr_dec.h"
#include    "e_sbr_error.h"

SBR_ERROR sbr_reset_dec(SBR_FRAME_DATA * hFrameData,
                        SBR_DEC * sbrDec,
                        Int32 upsampleFac);

#endif

