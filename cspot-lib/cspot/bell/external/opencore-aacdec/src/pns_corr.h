

#ifndef PNS_CORR_H
#define PNS_CORR_H

#include "pv_audio_type_defs.h"

void pns_corr(
    const Int scale,
    const Int coef_per_win,
    const Int sfb_per_win,
    const Int wins_in_group,
    const Int band_length,
    const Int q_formatLeft,
    Int q_formatRight[],
    const Int32 coefLeft[],
    Int32 coefRight[]);

#endif

