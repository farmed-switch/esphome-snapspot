

#ifndef PNS_LEFT_H
#define PNS_LEFT_H

#include "pv_audio_type_defs.h"
#include "s_frameinfo.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void pns_left(
        const FrameInfo *pFrameInfo,
        const Int        group[],
        const Int        codebook_map[],
        const Int        factors[],
        const Int        sfb_prediction_used[],
        const Bool       ltp_data_present,
        Int32      spectral_coef[],
        Int        q_format[],
        Int32     *pCurrentSeed);

#ifdef __cplusplus
}
#endif

#endif

