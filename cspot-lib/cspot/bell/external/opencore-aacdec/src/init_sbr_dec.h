

#ifndef INIT_SBR_DEC_H
#define INIT_SBR_DEC_H

#include "sbr_dec.h"

Int32 init_sbr_dec(Int32 codecSampleRate,
                   Int  upsampleFac,
                   SBR_DEC *sbrDec,
                   SBR_FRAME_DATA *hFrameData);

#endif

