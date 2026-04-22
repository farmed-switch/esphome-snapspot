

#ifndef PS_ALLOCATE_DECODER_H
#define PS_ALLOCATE_DECODER_H

#include "pv_audio_type_defs.h"

#ifdef __cplusplus
extern "C"
{
#endif

    Int32 ps_allocate_decoder(SBRDECODER_DATA *self,
    UInt32  noSubSamples);

#ifdef __cplusplus
}
#endif

#endif
