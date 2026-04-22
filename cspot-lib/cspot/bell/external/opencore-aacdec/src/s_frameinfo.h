

#ifndef S_FRAMEINFO_H
#define S_FRAMEINFO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "pv_audio_type_defs.h"
#include "e_blockswitching.h"

    typedef struct
    {
        Int     islong;
        Int     num_win;
        Int     coef_per_frame;
        Int     sfb_per_frame;
        Int     coef_per_win[MAX_WIN];
        Int     sfb_per_win[MAX_WIN];
        Int     sectbits[MAX_WIN];
        Int16   *win_sfb_top[MAX_WIN];
        Int     *sfb_width_128;

        Int     frame_sfb_top[MAXBANDS];
        Int     num_groups;
        Int     group_len[8];

    } FrameInfo;

#ifdef __cplusplus
}
#endif

#endif
