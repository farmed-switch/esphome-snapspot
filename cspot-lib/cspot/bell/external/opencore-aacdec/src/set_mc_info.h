

#ifndef SET_MC_INFO_H
#define SET_MC_INFO_H

#include    "pv_audio_type_defs.h"
#include    "s_mc_info.h"
#include    "s_frameinfo.h"

Int set_mc_info(
    MC_Info     *pMC_Info,
    const tMP4AudioObjectType objectType,
    const Int    sampling_rate_idx,
    const Int    tag,
    const Int    is_cpe,
    FrameInfo   *pWinSeqInfo[],
    Int          pSfbwidth128[]
);

#endif

