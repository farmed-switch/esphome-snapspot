

#include    "pv_audio_type_defs.h"
#include    "s_sr_info.h"
#include    "s_frameinfo.h"
#include    "e_blockswitching.h"
#include    "e_huffmanconst.h"
#include    "sfb.h"
#include    "huffman.h"

Int infoinit(
    const Int samp_rate_idx,
    FrameInfo   **ppWin_seq_info,
    Int    *pSfbwidth128)

{

    Int     i;
    Int     sfb_idx, sfb_sbk;
    Int     bins_sbk;
    Int     win_seq;
    Int     start_idx, end_idx;
    Int     nsfb_short;
    Int16   *sfbands;
    FrameInfo    *pFrameInfo;

    const SR_Info *pSi = &(samp_rate_info[samp_rate_idx]);

    const Int16 * pt_SFbands1024 = NULL;
    const Int16 * pt_SFbands128  = NULL;

    switch (pSi->samp_rate)
    {
        case 96000:
        case 88200:
            pt_SFbands1024  = sfb_96_1024;
            pt_SFbands128   = sfb_64_128;
            break;
        case 64000:
            pt_SFbands1024  = sfb_64_1024;
            pt_SFbands128   = sfb_64_128;
            break;
        case 48000:
        case 44100:
            pt_SFbands1024  = sfb_48_1024;
            pt_SFbands128   = sfb_48_128;
            break;
        case 32000:
            pt_SFbands1024  = sfb_32_1024;
            pt_SFbands128   = sfb_48_128;
            break;
        case 24000:
        case 22050:
            pt_SFbands1024  = sfb_24_1024;
            pt_SFbands128   = sfb_24_128;
            break;
        case 16000:
        case 12000:
        case 11025:
            pt_SFbands1024  = sfb_16_1024;
            pt_SFbands128   = sfb_16_128;
            break;
        case 8000:
            pt_SFbands1024  = sfb_8_1024;
            pt_SFbands128   = sfb_8_128;
            break;
        default:

            return -1;
    }

    pFrameInfo = ppWin_seq_info[ONLY_LONG_WINDOW];
    pFrameInfo->islong               = 1;
    pFrameInfo->num_win              = 1;
    pFrameInfo->coef_per_frame       = LN2;

    pFrameInfo->sfb_per_win[0]  = pSi->nsfb1024;
    pFrameInfo->sectbits[0]     = LONG_SECT_BITS;
    pFrameInfo->win_sfb_top[0]  = (Int16 *)pt_SFbands1024;

    pFrameInfo->sfb_width_128 = NULL;
    pFrameInfo->num_groups    = 1;
    pFrameInfo->group_len[0]  = 1;

    pFrameInfo = ppWin_seq_info[EIGHT_SHORT_WINDOW];
    pFrameInfo->islong                  = 0;
    pFrameInfo->num_win                 = NSHORT;
    pFrameInfo->coef_per_frame          = LN2;

    for (i = 0; i < pFrameInfo->num_win; i++)
    {
        pFrameInfo->sfb_per_win[i] = pSi->nsfb128;
        pFrameInfo->sectbits[i]    = SHORT_SECT_BITS;
        pFrameInfo->win_sfb_top[i] = (Int16 *)pt_SFbands128;
    }

    pFrameInfo->sfb_width_128 = pSfbwidth128;
    for (i = 0, start_idx = 0, nsfb_short = pSi->nsfb128; i < nsfb_short; i++)
    {
        end_idx = pt_SFbands128[i];
        pSfbwidth128[i] = end_idx - start_idx;
        start_idx = end_idx;
    }

    for (win_seq = 0; win_seq < NUM_WIN_SEQ; win_seq++)
    {

        if (ppWin_seq_info[win_seq] != NULL)
        {
            pFrameInfo                 = ppWin_seq_info[win_seq];
            pFrameInfo->sfb_per_frame  = 0;
            sfb_sbk                    = 0;
            bins_sbk                   = 0;

            for (i = 0; i < pFrameInfo->num_win; i++)
            {

                pFrameInfo->coef_per_win[i] =
                    pFrameInfo->coef_per_frame / pFrameInfo->num_win;

                pFrameInfo->sfb_per_frame += pFrameInfo->sfb_per_win[i];

                sfbands = pFrameInfo->win_sfb_top[i];
                for (sfb_idx = 0; sfb_idx < pFrameInfo->sfb_per_win[i];
                        sfb_idx++)
                {
                    pFrameInfo->frame_sfb_top[sfb_idx+sfb_sbk] =
                        sfbands[sfb_idx] + bins_sbk;
                }

                bins_sbk += pFrameInfo->coef_per_win[i];
                sfb_sbk  += pFrameInfo->sfb_per_win[i];
            }
        }

    }

    return SUCCESS;

}
