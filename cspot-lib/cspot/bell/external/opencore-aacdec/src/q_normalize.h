

#ifndef Q_NORMALIZE_H
#define Q_NORMALIZE_H

#include "pv_audio_type_defs.h"
#include "s_frameinfo.h"

#ifdef __cplusplus
extern "C"
{
#endif

    Int q_normalize(
        Int        qFormat[],
        const FrameInfo *pFrameInfo,
        Int32     abs_max_per_window[],
        Int32      coef[]);

#ifdef __cplusplus
}
#endif

#endif
