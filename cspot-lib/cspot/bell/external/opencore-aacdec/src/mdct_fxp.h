

#ifndef MDCT_FXP_H
#define MDCT_FXP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "pv_audio_type_defs.h"

#define     LONG_WINDOW_TYPE  2048
#define     SHORT_WINDOW_TYPE  256

    extern const Int exp_rotation_N_256[128];
    extern const Int exp_rotation_N_2048[1024];

    Int mdct_fxp(
        Int32   data_quant[],
        Int32   Q_FFTarray[],
        Int     n);

#ifdef __cplusplus
}
#endif

#endif
