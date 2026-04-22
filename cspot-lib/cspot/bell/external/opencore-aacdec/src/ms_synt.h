

#ifndef MS_SYNT_H
#define MS_SYNT_H

#include "pv_audio_type_defs.h"

void ms_synt(
    const Int   wins_in_group,
    const Int   coef_per_win,
    const Int   num_bands,
    const Int   band_length,
    Int32 spectralCoefLeft[],
    Int32 spectralCoefRight[],
    Int   q_formatLeft[],
    Int   q_formatRight[]);

#endif

