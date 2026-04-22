

#include    "pv_audio_type_defs.h"
#include    "huffman.h"
#include    "aac_mem_funcs.h"

void  calc_gsfb_table(
    FrameInfo   *pFrameInfo,
    Int         group[])
{

    Int      group_idx;
    Int      offset;
    Int     *pFrameSfbTop;
    Int     *pSfbWidth128;
    Int      sfb;
    Int      nsfb;
    Int      len;
    Int      ngroups;

    pv_memset(pFrameInfo->frame_sfb_top,
              0,
              MAXBANDS*sizeof(pFrameInfo->frame_sfb_top[0]));

    offset      = 0;
    ngroups     = 0;
    do
    {
        pFrameInfo->group_len[ngroups] = group[ngroups] - offset;
        offset = group[ngroups];
        ngroups++;

    }
    while (offset < NUM_SHORT_WINDOWS);

    pFrameInfo->num_groups = ngroups;

    pFrameSfbTop = pFrameInfo->frame_sfb_top;
    offset = 0;

    for (group_idx = 0; group_idx < ngroups; group_idx++)
    {
        len  = pFrameInfo->group_len[  group_idx];
        nsfb = pFrameInfo->sfb_per_win[group_idx];

        pSfbWidth128 = pFrameInfo->sfb_width_128;

        for (sfb = nsfb; sfb > 0; sfb--)
        {
            offset += *pSfbWidth128++ * len;
            *pFrameSfbTop++ = offset;
        }
    }

}

