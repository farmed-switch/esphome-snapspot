

#ifndef PS_ALL_PASS_FRACT_DELAY_FILTER_H
#define PS_ALL_PASS_FRACT_DELAY_FILTER_H

#include "pv_audio_type_defs.h"

#define R_SHIFT     29
#define Q29_fmt(x)   (Int32)(x*((Int32)1<<R_SHIFT) + (x>=0?0.5F:-0.5F))

#define Qfmt15(x)   (Int16)(x*((Int32)1<<15) + (x>=0?0.5F:-0.5F))

#ifdef __cplusplus
extern "C"
{
#endif

    void ps_all_pass_fract_delay_filter_type_I(UInt32 *delayBufIndex,
    Int32 sb_delay,
    const Int32 *ppFractDelayPhaseFactorSer,
    Int32 ***pppRealDelayRBufferSer,
    Int32 ***pppImagDelayRBufferSer,
    Int32 *rIn,
    Int32 *iIn);

    void ps_all_pass_fract_delay_filter_type_II(UInt32 *delayBufIndex,
            Int32 sb_delay,
            const Int32 *ppFractDelayPhaseFactorSer,
            Int32 ***pppRealDelayRBufferSer,
            Int32 ***pppImagDelayRBufferSer,
            Int32 *rIn,
            Int32 *iIn,
            Int32 decayScaleFactor);

#ifdef __cplusplus
}
#endif

#endif
