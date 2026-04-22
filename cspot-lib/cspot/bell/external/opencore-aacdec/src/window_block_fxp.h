

#ifndef WINDOW_BLOCK_FXP_H
#define WINDOW_BLOCK_FXP_H

#include "pv_audio_type_defs.h"
#include "e_window_shape.h"
#include "e_window_sequence.h"
#include "config.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define LONG_WINDOW         (1024)
#define SHORT_WINDOW        (128)

#define HALF_LONG_WINDOW    (LONG_WINDOW>>1)
#define HALF_SHORT_WINDOW   (SHORT_WINDOW>>1)

#define NUM_SHORT_WINDOWS   (8)
#define LONG_WINDOW_m_1     (LONG_WINDOW-1)
#define SHORT_WINDOW_m_1    (SHORT_WINDOW-1)

#define W_L_START_1         ((3*LONG_WINDOW - SHORT_WINDOW)>>1)
#define W_L_START_2         ((3*LONG_WINDOW + SHORT_WINDOW)>>1)
#define W_L_STOP_1          ((LONG_WINDOW - SHORT_WINDOW)>>1)
#define W_L_STOP_2          ((LONG_WINDOW + SHORT_WINDOW)>>1)

#define LONG_BLOCK1          (2*LONG_WINDOW)
#define SHORT_BLOCK1         (2*SHORT_WINDOW)

#define  SCALING    10
#define  ROUNDING     (1<<(SCALING-1))

    extern const Int16 Short_Window_KBD_fxp[ SHORT_WINDOW];
    extern const Int16 Long_Window_KBD_fxp[ LONG_WINDOW];
    extern const Int16 Short_Window_sine_fxp[ SHORT_WINDOW];
    extern const Int16 Long_Window_sine_fxp[ LONG_WINDOW];

    extern const Int16 * const Long_Window_fxp[];
    extern const Int16 * const Short_Window_fxp[];

    void trans4m_freq_2_time_fxp(
        Int32   Frequency_data[],
        Int32   Time_data[],
#ifdef AAC_PLUS
        Int32   Output_buffer[],
#else
        Int16   Output_buffer[],
#endif
        WINDOW_SEQUENCE wnd_seq,
        Int     wnd_shape_prev_bk,
        Int     wnd_shape_this_bk,
        Int     Q_format,
        Int32   abs_max_per_window[],
        Int32   freq_2_time_buffer[] ,
        Int16   *Interleave_output
    );

    void trans4m_freq_2_time_fxp_1(
        Int32   Frequency_data[],
        Int32   Time_data[],
        Int16   Output_buffer[],
        WINDOW_SEQUENCE wnd_seq,
        Int     wnd_shape_prev_bk,
        Int     wnd_shape_this_bk,
        Int     Q_format,
        Int32   abs_max_per_window[],
        Int32   freq_2_time_buffer[]
    );

    void trans4m_freq_2_time_fxp_2(
        Int32   Frequency_data[],
        Int32   Time_data[],
        WINDOW_SEQUENCE wnd_seq,
        Int     wnd_shape_prev_bk,
        Int     wnd_shape_this_bk,
        Int     Q_format,
        Int32   abs_max_per_window[],
        Int32   freq_2_time_buffer[] ,
        Int16   *Interleave_output
    );

    void trans4m_time_2_freq_fxp(
        Int32   Time2Freq_data[],
        WINDOW_SEQUENCE wnd_seq,
        Int     wnd_shape_prev_bk,
        Int     wnd_shape_this_bk,
        Int     *pQ_format,
        Int32   mem_4_in_place_FFT[]);

#ifdef __cplusplus
}
#endif

#endif

