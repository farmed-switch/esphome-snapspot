

#ifndef PS_STEREO_PROCESSING_H
#define PS_STEREO_PROCESSING_H

#include "pv_audio_type_defs.h"
#include "s_ps_dec.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void ps_stereo_processing(STRUCT_PS_DEC  *pms,
    Int32          *qmfLeftReal,
    Int32          *qmfLeftImag,
    Int32          *qmfRightReal,
    Int32          *qmfRightImag);

#ifdef __cplusplus
}
#endif

#endif
