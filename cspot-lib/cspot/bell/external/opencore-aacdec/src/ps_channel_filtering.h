

#ifndef PS_CHANNEL_FILTERING_H
#define PS_CHANNEL_FILTERING_H

#include "pv_audio_type_defs.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void two_ch_filtering(const Int32 *pQmf_r,
    const Int32 *pQmf_i,
    Int32 *mHybrid_r,
    Int32 *mHybrid_i);

    void eight_ch_filtering(const Int32 *pQmfReal,
                            const Int32 *pQmfImag,
                            Int32 *mHybridReal,
                            Int32 *mHybridImag,
                            Int32 scratch_mem[]);

#ifdef __cplusplus
}
#endif

#endif
