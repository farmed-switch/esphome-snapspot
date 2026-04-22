

#ifndef PS_HYBRID_ANALYSIS_H
#define PS_HYBRID_ANALYSIS_H

#include "pv_audio_type_defs.h"
#include "s_hybrid.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void ps_hybrid_analysis(const Int32 mQmfReal[][64],
    const Int32 mQmfImag[][64],
    Int32 *mHybridReal,
    Int32 *mHybridImag,
    HYBRID *pHybrid,
    Int32 scratch_mem[],
    Int32 band);
#ifdef __cplusplus
}
#endif

#endif
