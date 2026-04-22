

#ifndef PS_PWR_TRANSIENT_DETECTION_H
#define PS_PWR_TRANSIENT_DETECTION_H

#include "pv_audio_type_defs.h"
#include "s_ps_dec.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void ps_pwr_transient_detection(STRUCT_PS_DEC *h_ps_dec,
    Int32 *rIntBufferLeft,
    Int32 *iIntBufferLeft,
    Int32 aTransRatio[]);

#ifdef __cplusplus
}
#endif

#endif
