

#include "pv_audio_type_defs.h"
#include "s_bits.h"
#include "s_tdec_int_file.h"
#include "ibstream.h"
#include "set_mc_info.h"
#include "find_adts_syncword.h"
#include "get_adts_header.h"
#include "config.h"

#define LENGTH_VARIABLE_HEADER  28
#define LENGTH_FIXED_HEADER     28
#define LENGTH_SYNCWORD         15
#define LENGTH_CRC              16

#define ID_BIT_FILTER           0x7FFB
#define SYNCWORD_15BITS         0x7FF8
#define MASK_28BITS             0x0FFFFFFFL

Int get_adts_header(
    tDec_Int_File *pVars,
    UInt32        *pSyncword,
    Int           *pInvoke,
    Int            CorrectlyReadFramesCount)
{
    UInt32 adts_header;
    UInt   lower_16;
    Int    status = SUCCESS;
    UInt   channel_configuration;

    if (*(pInvoke) > CorrectlyReadFramesCount)
    {

        status = find_adts_syncword(pSyncword,
                                    &(pVars->inputStream),
                                    LENGTH_FIXED_HEADER,
                                    MASK_28BITS);
    }
    else
    {

        *(pSyncword) = SYNCWORD_15BITS;

        status = find_adts_syncword(pSyncword,
                                    &(pVars->inputStream),
                                    LENGTH_SYNCWORD,
                                    ID_BIT_FILTER);

        adts_header = getbits((LENGTH_FIXED_HEADER - LENGTH_SYNCWORD),
                              &(pVars->inputStream));

        *(pSyncword) <<= (LENGTH_FIXED_HEADER - LENGTH_SYNCWORD);
        *(pSyncword)  |= adts_header;

        pVars->prog_config.CRC_absent  = ((UInt)(adts_header >> 12)) & 0x0001;

        lower_16 = (UInt)adts_header;

        pVars->prog_config.profile = (lower_16 >> 10) & 0x3;

        if (pVars->prog_config.profile == MP4AUDIO_AAC_SSR)
        {
            status = 1;
        }

        pVars->prog_config.sampling_rate_idx = (lower_16 >> 6) & 0xF;

        channel_configuration = (lower_16 >> 2) & 0x7;

        if (channel_configuration > 2)
        {
            status = 1;
        }

        if (channel_configuration)
        {
            channel_configuration--;
        }
        pVars->prog_config.front.ele_is_cpe[0] = channel_configuration;

        pVars->prog_config.front.num_ele    = 1;

        pVars->prog_config.front.ele_tag[0] = 0;

        pVars->prog_config.mono_mix.present = 0;
        pVars->prog_config.stereo_mix.present = 0;
        pVars->prog_config.matrix_mix.present = 0;

        if (status == SUCCESS)
        {

            status =
                set_mc_info(
                    &(pVars->mc_info),
                    (tMP4AudioObjectType)(pVars->prog_config.profile + 1),
                    pVars->prog_config.sampling_rate_idx,
                    pVars->prog_config.front.ele_tag[0],
                    pVars->prog_config.front.ele_is_cpe[0],
                    pVars->winmap,
                    pVars->SFBWidth128);

        }

#ifdef AAC_PLUS

        if ((pVars->prog_config.sampling_rate_idx >= 6) && (pVars->aacPlusEnabled == TRUE))
        {
            pVars->mc_info.upsamplingFactor = 2;
            pVars->prog_config.sampling_rate_idx -= 3;
            pVars->mc_info.sbrPresentFlag = 1;
            pVars->sbrDecoderData.SbrChannel[0].syncState = SBR_ACTIVE;
            pVars->sbrDecoderData.SbrChannel[1].syncState = SBR_ACTIVE;
        }
#endif

        if (status == SUCCESS)
        {
            (*pInvoke)++;
        }
        else
        {
            (*pInvoke) = 0;
        }

    }

    adts_header = getbits(
                      LENGTH_VARIABLE_HEADER,
                      &(pVars->inputStream));

    pVars->prog_config.frame_length  = ((UInt)(adts_header >> 13)) & 0x1FFF;

    lower_16 = (UInt)adts_header;

    pVars->prog_config.buffer_fullness = (lower_16 >> 2) & 0x7FF;

    pVars->prog_config.headerless_frames = (lower_16 & 0x0003);

    if (pVars->prog_config.CRC_absent == 0)
    {
        pVars->prog_config.CRC_check = (UInt)getbits(
                                           LENGTH_CRC,
                                           &(pVars->inputStream));
    }

    return (status);

}
