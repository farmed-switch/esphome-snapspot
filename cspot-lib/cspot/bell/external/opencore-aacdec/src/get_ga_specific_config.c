

#include    "pv_audio_type_defs.h"
#include    "e_mp4ff_const.h"
#include    "e_tmp4audioobjecttype.h"
#include    "s_tdec_int_file.h"
#include    "get_ga_specific_config.h"
#include    "set_mc_info.h"
#include    "get_prog_config.h"
#include    "ibstream.h"

Int get_GA_specific_config(
    tDec_Int_File * const pVars,
    BITS    *pInputStream,
    UInt     channel_config,
    const tMP4AudioObjectType audioObjectType
)
{

    Int status = SUCCESS;
    UInt dependsOnCoreCoder;

    UInt extFlag;

    Int  extFlag3;

    get1bits(
        pInputStream);

    dependsOnCoreCoder =
        get1bits(
            pInputStream);

    if (dependsOnCoreCoder != FALSE)
    {

        status = 1;
    }

    extFlag = get1bits(pInputStream);

    pVars->mc_info.implicit_channeling = 1;

    if (status == SUCCESS)
    {

        if (channel_config == 0)
        {
            status = get_prog_config(pVars,
                                     &pVars->scratch.scratch_prog_config);

            if (status != SUCCESS)
            {
                pVars->prog_config.front.ele_is_cpe[0] = 0;
                pVars->mc_info.nch = 1;
                pVars->prog_config.front.ele_tag[0] = 0;

                status = SUCCESS;
            }
        }
        else
        {

            channel_config--;
            pVars->prog_config.front.ele_is_cpe[0] = channel_config;
            pVars->prog_config.front.ele_tag[0] = 0;

            status =
                set_mc_info(
                    &(pVars->mc_info),
                    audioObjectType,
                    pVars->prog_config.sampling_rate_idx,
                    pVars->prog_config.front.ele_tag[0],
                    pVars->prog_config.front.ele_is_cpe[0],
                    pVars->winmap,
                    pVars->SFBWidth128);

        }

    }

    if ((audioObjectType == MP4AUDIO_AAC_SCALABLE) ||
            (audioObjectType == MP4AUDIO_ER_AAC_SCALABLE))
    {

        status = 1;
    }

    if (extFlag)
    {

        if (audioObjectType == MP4AUDIO_ER_BSAC)
        {
            status = 1;

        }

        if (((audioObjectType > 16) && (audioObjectType < 22)) ||
                (audioObjectType == 23))
        {
            status = 1;

        }

        extFlag3 = get1bits(pInputStream);

        if (extFlag3)
        {
            status = 1;
        }

    }

    return status;
}
