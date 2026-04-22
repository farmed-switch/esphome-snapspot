

#include    "pv_audio_type_defs.h"
#include    "e_mp4ff_const.h"
#include    "e_tmp4audioobjecttype.h"
#include    "get_audio_specific_config.h"
#include    "get_ga_specific_config.h"
#include    "ibstream.h"
#include    "sfb.h"
#include    "config.h"

Int get_audio_specific_config(tDec_Int_File   * const pVars)
{

    UInt    temp;
    tMP4AudioObjectType     audioObjectType;

    UInt    channel_config;
    UInt    syncExtensionType;
    UInt    extensionAudioObjectType = 0;
    UInt    extensionSamplingFrequencyIndex = 0;
    BITS   *pInputStream;
    Int     status;

    status = SUCCESS;

    pInputStream = &(pVars->inputStream);

    pVars->mc_info.upsamplingFactor = 1;

    temp =  get9_n_lessbits(LEN_OBJ_TYPE + LEN_SAMP_RATE_IDX,
                            pInputStream);

    audioObjectType = (tMP4AudioObjectType)((temp & 0x1f0) >> 4);

    pVars->mc_info.ExtendedAudioObjectType =  audioObjectType;

    pVars->prog_config.sampling_rate_idx = temp & 0xf;

    if (pVars->prog_config.sampling_rate_idx > 0xb)
    {

        if (pVars->prog_config.sampling_rate_idx == 0xf)
        {

            getbits(LEN_SAMP_RATE, pInputStream);
        }

        status = 1;
    }

    channel_config =  get9_n_lessbits(LEN_CHAN_CONFIG,
                                      pInputStream);

    if ((channel_config > 2) && (!pVars->aacConfigUtilityEnabled))
    {

        status = 1;

    }

    if (audioObjectType == MP4AUDIO_SBR || audioObjectType == MP4AUDIO_PS)
    {

        pVars->mc_info.ExtendedAudioObjectType = MP4AUDIO_SBR;
        pVars->mc_info.sbrPresentFlag = 1;

        if (audioObjectType == MP4AUDIO_PS)
        {
            pVars->mc_info.psPresentFlag = 1;
            pVars->mc_info.ExtendedAudioObjectType = MP4AUDIO_PS;
        }

        extensionSamplingFrequencyIndex =
            get9_n_lessbits(LEN_SAMP_RATE_IDX,
                            pInputStream);
        if (extensionSamplingFrequencyIndex == 0x0f)
        {

            getbits(LEN_SAMP_RATE, pInputStream);
        }

        audioObjectType = (tMP4AudioObjectType) get9_n_lessbits(LEN_OBJ_TYPE ,
                          pInputStream);
    }

    if ((
                (audioObjectType == MP4AUDIO_AAC_LC)        ||

                (audioObjectType == MP4AUDIO_LTP)

                 ) && (status == SUCCESS))
    {
        status = get_GA_specific_config(pVars,
                                        pInputStream,
                                        channel_config,
                                        audioObjectType);

        if ((pVars->mc_info.audioObjectType != MP4AUDIO_AAC_LC) &&
                (pVars->mc_info.audioObjectType != MP4AUDIO_LTP))
        {
            return 1;
        }
    }
    else
    {
        return 1;
    }

    if (extensionAudioObjectType != MP4AUDIO_SBR)
    {
        syncExtensionType = (UInt)get17_n_lessbits(LEN_SYNC_EXTENSION_TYPE,
                            pInputStream);

        if (syncExtensionType == 0x2b7)
        {
            extensionAudioObjectType = get9_n_lessbits(
                                           LEN_OBJ_TYPE,
                                           pInputStream);

            if (extensionAudioObjectType == MP4AUDIO_SBR)
            {
                pVars->mc_info.sbrPresentFlag = get1bits(pInputStream);
                if (pVars->mc_info.sbrPresentFlag == 1)
                {
                    extensionSamplingFrequencyIndex =
                        get9_n_lessbits(
                            LEN_SAMP_RATE_IDX,
                            pInputStream);
                    if (pVars->aacPlusEnabled == TRUE)
                    {
#ifdef AAC_PLUS
                        pVars->mc_info.upsamplingFactor = (samp_rate_info[extensionSamplingFrequencyIndex].samp_rate >> 1) ==
                                                          samp_rate_info[pVars->prog_config.sampling_rate_idx].samp_rate ? 2 : 1;

                        if ((Int)extensionSamplingFrequencyIndex == pVars->prog_config.sampling_rate_idx)
                        {

                            if (pVars->prog_config.sampling_rate_idx < 6)
                            {
                                pVars->aacPlusEnabled = FALSE;
                            }

                            pVars->mc_info.bDownSampledSbr = TRUE;
                        }
                        pVars->prog_config.sampling_rate_idx = extensionSamplingFrequencyIndex;

#endif
                    }

                    if (extensionSamplingFrequencyIndex == 0x0f)
                    {

                        getbits(LEN_SAMP_RATE, pInputStream);
                    }

                    syncExtensionType = (UInt)get17_n_lessbits(LEN_SYNC_EXTENSION_TYPE,
                                        pInputStream);
                    if (syncExtensionType == 0x548)
                    {
                        pVars->mc_info.psPresentFlag = get1bits(pInputStream);
                        if (pVars->mc_info.psPresentFlag)
                        {
                            extensionAudioObjectType = MP4AUDIO_PS;
                        }
                    }
                    else
                    {

                        pVars->inputStream.usedBits -= LEN_SYNC_EXTENSION_TYPE;
                    }

                    pVars->mc_info.ExtendedAudioObjectType = (  tMP4AudioObjectType)extensionAudioObjectType;
                }
            }
        }
        else if (!status)
        {

            pVars->inputStream.usedBits -= LEN_SYNC_EXTENSION_TYPE;

#ifdef AAC_PLUS

            if ((pVars->prog_config.sampling_rate_idx >= 6) && (pVars->aacPlusEnabled == TRUE) &&
                    audioObjectType == MP4AUDIO_AAC_LC)
            {
                pVars->mc_info.upsamplingFactor = 2;
                pVars->prog_config.sampling_rate_idx -= 3;
                pVars->mc_info.sbrPresentFlag = 1;
                pVars->sbrDecoderData.SbrChannel[0].syncState = SBR_NOT_INITIALIZED;
                pVars->sbrDecoderData.SbrChannel[1].syncState = SBR_NOT_INITIALIZED;

            }
#endif

        }
    }
    else
    {

        if (pVars->aacPlusEnabled == TRUE)
        {
#ifdef AAC_PLUS
            pVars->mc_info.upsamplingFactor = (samp_rate_info[extensionSamplingFrequencyIndex].samp_rate >> 1) ==
                                              samp_rate_info[pVars->prog_config.sampling_rate_idx].samp_rate ? 2 : 1;

            if ((Int)extensionSamplingFrequencyIndex == pVars->prog_config.sampling_rate_idx)
            {

                if (pVars->prog_config.sampling_rate_idx < 6)
                {
                    pVars->aacPlusEnabled = FALSE;
                }
                pVars->mc_info.bDownSampledSbr = TRUE;
            }
            pVars->prog_config.sampling_rate_idx = extensionSamplingFrequencyIndex;

#endif

        }

    }

    return status;

}
