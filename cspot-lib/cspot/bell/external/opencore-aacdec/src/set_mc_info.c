

#include    "pv_audio_type_defs.h"
#include    "set_mc_info.h"
#include    "huffman.h"
#include    "s_ch_info.h"

Int set_mc_info(
    MC_Info     *pMC_Info,
    const tMP4AudioObjectType audioObjectType,
    const Int    sampling_rate_idx,
    const Int    tag,
    const Int    is_cpe,
    FrameInfo   *pWinSeqInfo[],
    Int          sfbwidth128[]
)
{
    Ch_Info *pCh_Info;

    pMC_Info->audioObjectType = audioObjectType;

    if (pMC_Info->sampling_rate_idx != sampling_rate_idx)
    {
        pMC_Info->sampling_rate_idx = sampling_rate_idx;

        Int status;
        status = infoinit(sampling_rate_idx,
                          pWinSeqInfo,
                          sfbwidth128);
        if (SUCCESS != status)
        {
            return 1;
        }
    }

    pMC_Info->nch   = 1 + is_cpe;

    pCh_Info = &pMC_Info->ch_info[0];
    pCh_Info->tag = tag;
    pCh_Info->cpe = is_cpe;

    if (is_cpe != FALSE)
    {

        pCh_Info = &pMC_Info->ch_info[1];
        pCh_Info->cpe = TRUE;

    }

    return(SUCCESS);
}
