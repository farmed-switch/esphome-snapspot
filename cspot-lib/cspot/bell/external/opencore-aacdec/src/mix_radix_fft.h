

#ifndef MIX_RADIX_FFT_H
#define MIX_RADIX_FFT_H

#include "pv_audio_type_defs.h"

#define     FFT_RX4_LENGTH_FOR_LONG         512
#define     HALF_FFT_RX4_LENGTH_FOR_LONG    (FFT_RX4_LENGTH_FOR_LONG>>1)
#define     ONE_FOURTH_FFT_RX4_LENGTH_FOR_LONG   (FFT_RX4_LENGTH_FOR_LONG>>2)

#ifdef __cplusplus
extern "C"
{
#endif

    Int mix_radix_fft(
        Int32   *Data,
        Int32   *peak_value);

#ifdef __cplusplus
}
#endif

#endif
