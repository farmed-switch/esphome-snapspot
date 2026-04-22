

#ifndef APPLY_MS_SYNT_H
#define APPLY_MS_SYNT_H

#include "pv_audio_type_defs.h"
#include "s_frameinfo.h"

void apply_ms_synt(
    const FrameInfo * const pFrameInfo,
    const Int        group[],
    const Bool       mask_map[],
    const Int        codebook_map[],
    Int32      coefLeft[],
    Int32      coefRight[],
    Int        q_formatLeft[MAXBANDS],
    Int        q_formatRight[MAXBANDS]);

#endif

