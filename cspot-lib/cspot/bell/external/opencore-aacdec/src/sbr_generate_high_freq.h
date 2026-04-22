

#ifndef SBR_GENERATE_HIGH_FREQ_H
#define SBR_GENERATE_HIGH_FREQ_H

#include    "e_invf_mode.h"
#include    "s_patch.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void sbr_generate_high_freq(
        Int32 sourceBufferReal[][32],
        Int32 sourceBufferImag[][32],
        Int32 *targetBufferReal,
        Int32 *targetBufferImag,
        INVF_MODE *invFiltMode,
        INVF_MODE *prevInvFiltMode,
        Int32 *invFiltBandTable,
        Int32 noInvFiltBands,
        Int32 highBandStartSb,
        Int32 *v_k_master,
        Int32 numMaster,
        Int32 fs,
        Int32 *frameInfo,
        Int32 *degreeAlias,
        Int32 scratch_mem[][64],
        Int32 BwVector[MAX_NUM_PATCHES],
        Int32 BwVectorOld[MAX_NUM_PATCHES],
        struct PATCH * Patch,
        Int32 LC_flag,
        Int32 *highBandStopSb);

#ifdef __cplusplus
}
#endif

#endif

