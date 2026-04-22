

#ifndef FFT_RX4_H
#define FFT_RX4_H

#include "pv_audio_type_defs.h"

#define     FFT_RX4_LONG                256
#define     ONE_FOURTH_FFT_RX4_LONG     ((FFT_RX4_LONG)>>2)
#define     FFT_RX4_SHORT               64
#define     ONE_FOURTH_FFT_RX4_SHORT    ((FFT_RX4_SHORT)>>2)

extern const Int16 w_64rx4[];
extern const Int32 W_64rx4[];
extern const Int32 W_256rx4[];
extern const Int32 w_512rx2[];

#ifdef __cplusplus
extern "C"
{
#endif

    void fft_rx4_long(
        Int32      Data[],
        Int32      *peak_value);

    Int fft_rx4_short(
        Int32      Data[],
        Int32      *peak_value);

#ifdef __cplusplus
}
#endif

#endif
