

#ifndef ANALYSIS_SUB_BAND_H
#define ANALYSIS_SUB_BAND_H

#include "pv_audio_type_defs.h"
#include "config.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void analysis_sub_band_LC(Int32 vec[64],
    Int32 cosine_total[],
    Int32 maxBand,
    Int32 scratch_mem[][64]);

#ifdef HQ_SBR

    void analysis_sub_band(Int32 vec[64],
                           Int32 cosine_total[],
                           Int32 sine_total[],
                           Int32 maxBand,
                           Int32 scratch_mem[][64]);

#endif

#ifdef __cplusplus
}
#endif

#endif
