

#ifndef SBR_DOWNSAMPLE_LO_RES_H
#define SBR_DOWNSAMPLE_LO_RES_H

#include "pv_audio_type_defs.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void  sbr_downsample_lo_res(Int32 v_result[],
    Int32 num_result,
    Int   freqBandTableRef[],
    Int32 num_Ref);

#ifdef __cplusplus
}
#endif

#endif

