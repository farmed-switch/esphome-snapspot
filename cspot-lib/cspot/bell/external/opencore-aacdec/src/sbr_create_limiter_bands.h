

#ifndef SBR_CREATE_LIMITER_BANDS_H
#define SBR_CREATE_LIMITER_BANDS_H

#include    "pv_audio_type_defs.h"
#include    "sbr_generate_high_freq.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void sbr_create_limiter_bands(Int32 limSbc[4][12 + 1],
    Int32 gateMode[4],
    Int   *freqTable,
    struct PATCH Patch,
    const Int32 noBands);

#ifdef __cplusplus
}
#endif

#endif

