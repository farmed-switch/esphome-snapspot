

#ifndef PS_HYBRID_SYNTHESIS_H
#define PS_HYBRID_SYNTHESIS_H

#include "pv_audio_type_defs.h"
#include "s_hybrid.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void ps_hybrid_synthesis(const Int32 *mHybridReal,
    const Int32 *mHybridImag,
    Int32 *mQmfReal,
    Int32 *mQmfImag,
    HYBRID *hHybrid);

#ifdef __cplusplus
}
#endif

#endif
