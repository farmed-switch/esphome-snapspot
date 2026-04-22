

#ifndef S_TNSFILT_H
#define S_TNSFILT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "pv_audio_type_defs.h"
#include "e_tns_const.h"

    typedef struct
    {
        Int start_band;
        Int stop_band;
        Int start_coef;
        Int stop_coef;
        UInt order;
        Int direction;
        Int q_lpc;

    } TNSfilt;

#ifdef __cplusplus
}
#endif

#endif
