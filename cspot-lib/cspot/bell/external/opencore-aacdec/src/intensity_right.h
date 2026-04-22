

#ifndef INTENSITY_RIGHT_H
#define INTENSITY_RIGHT_H

#include "pv_audio_type_defs.h"

void intensity_right(
    const Int   scalefactor,
    const Int   coef_per_win,
    const Int   sfb_per_win,
    const Int   wins_in_group,
    const Int   band_length,
    const Int   codebook,
    const Bool  ms_used,
    const Int   q_formatLeft[],
    Int   q_formatRight[],
    const Int32 coefLeft[],
    Int32 coefRight[]);

#endif

