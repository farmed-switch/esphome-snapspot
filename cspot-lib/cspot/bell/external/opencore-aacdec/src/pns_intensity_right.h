

#ifndef PNS_INTENSITY_RIGHT_H
#define PNS_INTENSITY_RIGHT_H

#include    "pv_audio_type_defs.h"
#include    "s_frameinfo.h"

void pns_intensity_right(
    const Int        hasmask,
    const FrameInfo * const pFrameInfo,
    const Int        group[],
    const Bool       mask_map[],
    const Int        codebook_map[],
    const Int        factorsL[],
    const Int        factorsR[],
    Int        sfb_prediction_used[],
    const Bool       ltp_data_present,
    Int32      spectralCoefLeft[],
    Int32      spectralCoefRight[],
    Int        q_formatLeft[MAXBANDS],
    Int        q_formatRight[MAXBANDS],
    Int32     * const pCurrentSeed);

#endif

