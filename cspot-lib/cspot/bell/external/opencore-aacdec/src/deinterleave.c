

#include    "pv_audio_type_defs.h"
#include    "huffman.h"
#include    "aac_mem_funcs.h"

void deinterleave(
    Int16        interleaved[],
    Int16        deinterleaved[],
    FrameInfo   *pFrameInfo)
{

    Int      group;
    Int      sfb;
    Int      win;
    Int16    *pGroup;
    Int16    *pWin;
    Int16    *pStart;
    Int16    *pInterleaved;
    Int16    *pDeinterleaved;
    Int      sfb_inc;

    Int      ngroups;
    Int     *pGroupLen;
    Int     *pSfbPerWin;
    Int     *pSfbWidth;

    pInterleaved   = interleaved;
    pDeinterleaved = deinterleaved;

    pSfbPerWin  = pFrameInfo->sfb_per_win;
    ngroups     = pFrameInfo->num_groups;
    pGroupLen   = pFrameInfo->group_len;

    pGroup = pDeinterleaved;

    for (group = ngroups; group > 0; group--)
    {
        pSfbWidth   = pFrameInfo->sfb_width_128;
        sfb_inc = 0;
        pStart = pInterleaved;

        for (sfb = pSfbPerWin[ngroups-group]; sfb > 0; sfb--)
        {
            pWin = pGroup;

            for (win = pGroupLen[ngroups-group]; win > 0; win--)
            {
                pDeinterleaved = pWin + sfb_inc;

                pv_memcpy(
                    pDeinterleaved,
                    pInterleaved,
                    *pSfbWidth*sizeof(*pInterleaved));

                pInterleaved += *pSfbWidth;

                pWin += SN2;

            }

            sfb_inc += *pSfbWidth++;

        }

        pGroup += (pInterleaved - pStart);

    }

}
