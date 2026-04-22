

#ifndef SYNTHESIS_SUB_BAND_H
#define SYNTHESIS_SUB_BAND_H

#include "pv_audio_type_defs.h"
#include "config.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void synthesis_sub_band_LC(Int32 Sr[], Int16 data[]);
    void synthesis_sub_band_LC_down_sampled(Int32 Sr[], Int16 data[]);

#ifdef HQ_SBR

    void synthesis_sub_band(Int32 Sr[], Int32 Si[], Int16 data[]);
    void synthesis_sub_band_down_sampled(Int32 Sr[], Int32 Si[], Int16 data[]);

#endif

#ifdef __cplusplus
}
#endif

#endif

