

#include "pv_audio_type_defs.h"
#include "s_tdec_int_file.h"
#include "ibstream.h"
#include "sfb.h"

#include "get_audio_specific_config.h"
#include "pvmp4audiodecoder_api.h"
#include "set_mc_info.h"
#include "config.h"

Int PVMP4SetAudioConfig(
    tPVMP4AudioDecoderExternal  *pExt,
    void                        *pMem,
    Int                         upsamplingFactor,
    Int                         samp_rate,
    Int                         num_ch,
    tMP4AudioObjectType         audioObjectType)

{

    tDec_Int_File *pVars;

    Int            status = MP4AUDEC_INCOMPLETE_FRAME;

    pVars = (tDec_Int_File *)pMem;

    pVars->inputStream.pBuffer = pExt->pInputBuffer;

    pVars->inputStream.availableBits = 0;

    pVars->inputStream.usedBits = 0;

    switch (samp_rate)
    {
        case 96000:
            pVars->prog_config.sampling_rate_idx = 0;
            break;
        case 88200:
            pVars->prog_config.sampling_rate_idx = 1;
            break;
        case 64000:
            pVars->prog_config.sampling_rate_idx = 2;
            break;
        case 48000:
            pVars->prog_config.sampling_rate_idx = 3;
            break;
        case 44100:
            pVars->prog_config.sampling_rate_idx = 4;
            break;
        case 32000:
            pVars->prog_config.sampling_rate_idx = 5;
            break;
        case 24000:
            pVars->prog_config.sampling_rate_idx = 6;
            break;
        case 22050:
            pVars->prog_config.sampling_rate_idx = 7;
            break;
        case 16000:
            pVars->prog_config.sampling_rate_idx = 8;
            break;
        case 12000:
            pVars->prog_config.sampling_rate_idx = 9;
            break;
        case 11025:
            pVars->prog_config.sampling_rate_idx = 10;
            break;
        case 8000:
            pVars->prog_config.sampling_rate_idx = 11;
            break;
        case 7350:
            pVars->prog_config.sampling_rate_idx = 12;
            break;
        default:
            status = -1;

            break;
    }

    pVars->mc_info.sbrPresentFlag = 0;
    pVars->mc_info.psPresentFlag = 0;
#ifdef AAC_PLUS
    pVars->mc_info.bDownSampledSbr = 0;
#endif
    pVars->mc_info.implicit_channeling = 0;
    pVars->mc_info.nch = num_ch;
    pVars->mc_info.upsamplingFactor = upsamplingFactor;

    if (num_ch == 2)
    {
        pVars->prog_config.front.ele_is_cpe[0] = 1;
    }
    else if (num_ch == 1)
    {
        pVars->prog_config.front.ele_is_cpe[0] = 0;
    }
    else
    {
        status = -1;
        pVars->status = status;
        return (status);
    }

    if ((audioObjectType == MP4AUDIO_AAC_LC)        ||
            (audioObjectType == MP4AUDIO_LTP))
    {
        pVars->aacPlusEnabled = FALSE;

        status = set_mc_info(&(pVars->mc_info),
                             audioObjectType,
                             pVars->prog_config.sampling_rate_idx,
                             pVars->prog_config.front.ele_tag[0],
                             pVars->prog_config.front.ele_is_cpe[0],
                             pVars->winmap,
                             pVars->SFBWidth128);
    }
    else if ((audioObjectType == MP4AUDIO_SBR)        ||
             (audioObjectType == MP4AUDIO_PS))
    {
        pVars->aacPlusEnabled = TRUE;

        status = set_mc_info(&(pVars->mc_info),
                             MP4AUDIO_AAC_LC,
                             pVars->prog_config.sampling_rate_idx,
                             pVars->prog_config.front.ele_tag[0],
                             pVars->prog_config.front.ele_is_cpe[0],
                             pVars->winmap,
                             pVars->SFBWidth128);

        pVars->mc_info.sbrPresentFlag = 1;
        if (audioObjectType == MP4AUDIO_PS)
        {
            pVars->mc_info.psPresentFlag = 1;
        }

        if (upsamplingFactor == 1)
        {
#ifdef AAC_PLUS
            pVars->mc_info.bDownSampledSbr = 1;
#endif

            if (pVars->prog_config.sampling_rate_idx < 6)
            {
                pVars->aacPlusEnabled = FALSE;
            }
        }

    }
    else
    {
        status = -1;
    }

    pExt->inputBufferUsedLength = 0;

    pExt->remainderBits = 0;

    pVars->bno++;

    pExt->samplingRate = samp_rate * upsamplingFactor;

    pExt->aacPlusEnabled = pVars->aacPlusEnabled;

    pExt->encodedChannels = 2;

    pExt->frameLength = 1024;
#ifdef AAC_PLUS
    pExt->aacPlusUpsamplingFactor = upsamplingFactor;
#endif

    pVars->status = status;

    return (status);

}
