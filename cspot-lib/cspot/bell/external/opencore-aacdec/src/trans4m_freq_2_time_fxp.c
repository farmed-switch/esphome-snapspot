

#include "config.h"

#include "pv_audio_type_defs.h"
#include "aac_mem_funcs.h"
#include "window_block_fxp.h"
#include "imdct_fxp.h"

#include "fxp_mul32.h"

#define  ROUNDING_SCALED     (ROUNDING<<(16 - SCALING))

#if defined(PV_ARM_V5)

static inline Int16 sat(Int32 y)
{
    Int32 x;
    Int32 z;
    __asm
    {
        mov x, ROUNDING_SCALED
        mov y, y, lsl #(15-SCALING)
        qdadd z, x, y
        mov y, z, lsr #16
    }
    return((Int16)y);
}

#define  limiter( y, x)   y = sat(x);

#elif defined(PV_ARM_GCC_V5)

static inline Int16 sat(Int32 y)
{
    register Int32 x;
    register Int32 ra = (Int32)y;
    register Int32 z = ROUNDING_SCALED;

    asm volatile(
        "mov %0, %1, lsl #5\n\t"
        "qdadd %0, %2, %0\n\t"
        "mov %0, %0, lsr #16"
    : "=&r*i"(x)
                : "r"(ra),
                "r"(z));

    return ((Int16)x);
}

#define  limiter( y, x)   y = sat(x);

#elif defined(PV_ARM_MSC_EVC_V5)

#define  limiter( y, x)       z = x<< (15-SCALING); \
                              y = _DAddSatInt( ROUNDING_SCALED, z)>>16;

#else

#define  limiter( y, x)   z = ((x + ROUNDING )>>SCALING); \
                          if ((z>>15) != (z>>31))         \
                          {                                    \
                              z = (z >> 31) ^ INT16_MAX;       \
                          } \
                          y = (Int16)(z);

#endif

#include    "config.h"

#ifdef AAC_PLUS

void trans4m_freq_2_time_fxp_1(
    Int32               Frequency_data[],
    Int32               Time_data[],
    Int16               Output_buffer[],
    WINDOW_SEQUENCE     wnd_seq,
    Int                 wnd_shape_prev_bk,
    Int                 wnd_shape_this_bk,
    Int                 Q_format,
    Int32               abs_max_per_window[],
    Int32               freq_2_time_buffer[])

{
    Int exp;
    Int shift;

    Int  i;
    Int  wnd;
#if !(defined( PV_ARM_GCC_V5)||(PV_ARM_V5))
    Int32 z;
#endif

    Int16 *pFreqInfo;
    Int32 temp;
    Int32 test;

    Int16 *pFreq_2_Time_data_1;
    Int16 *pFreq_2_Time_data_2;

    const Int16 *pLong_Window_1;
    const Int16 *pLong_Window_2;
    const Int16 *pShort_Window_1;
    const Int16 *pShort_Window_2;

    Int32 *pOverlap_and_Add_Buffer_1;
    Int32 *pOverlap_and_Add_Buffer_2;

    Int16  *pOutput_buffer;
    Int16  *pOutput_buffer_2;

    const Int16 * Long_Window_fxp[NUM_WINDOW_SHAPES];
    const Int16 * Short_Window_fxp[NUM_WINDOW_SHAPES];

    Long_Window_fxp[0] = Long_Window_sine_fxp;
    Long_Window_fxp[1] = Long_Window_KBD_fxp;
    Short_Window_fxp[0] = Short_Window_sine_fxp;
    Short_Window_fxp[1] = Short_Window_KBD_fxp;

    if (wnd_seq != EIGHT_SHORT_SEQUENCE)
    {

        pFreqInfo = (Int16 *)Frequency_data;

        exp = imdct_fxp(
                  (Int32 *)pFreqInfo,
                  freq_2_time_buffer,
                  LONG_BLOCK1,
                  Q_format,
                  abs_max_per_window[0]);

        if (exp < 16)
        {

            pFreq_2_Time_data_1 = pFreqInfo;

            switch (wnd_seq)
            {

                case ONLY_LONG_SEQUENCE:
                default:

                    pOutput_buffer = Output_buffer;

                    pOverlap_and_Add_Buffer_1 = Time_data;

                    {
                        const Int16 *pLong_Window_2 = &Long_Window_fxp[wnd_shape_this_bk][LONG_WINDOW_m_1];

                        Int32 * pFreq2T = (Int32 *)pFreqInfo;
                        Int32 * win = (Int32 *) & Long_Window_fxp[wnd_shape_prev_bk][0];
                        Int shift = exp + 15 - SCALING;

                        Int32 * pFreq2T_2 = &pFreq2T[HALF_LONG_WINDOW];

                        for (i = HALF_LONG_WINDOW; i != 0; i--)
                        {
                            Int16 win1, win2;
                            Int32  temp2, test2;

                            Int32  winx;

                            temp2 = *(pFreq2T++);
                            winx = *(win++);

                            test  = *(pOverlap_and_Add_Buffer_1++);
                            test2 = *(pOverlap_and_Add_Buffer_1--);
                            temp  =   fxp_mul_16_by_16bb(temp2, winx) >> shift;
                            temp2 =   fxp_mul_16_by_16tt(temp2, winx) >> shift;
                            limiter(*(pOutput_buffer++), (temp + test));
                            limiter(*(pOutput_buffer++), (temp2 + test2));

                            temp2 = *(pFreq2T_2++);

                            win1  = *(pLong_Window_2--);
                            win2  = *(pLong_Window_2--);
                            temp  = fxp_mul_16_by_16bb(temp2, win1) >> shift;
                            test2 = fxp_mul_16_by_16tb(temp2, win2) >> shift;
                            *(pOverlap_and_Add_Buffer_1++) = temp;
                            *(pOverlap_and_Add_Buffer_1++) = test2;

                        }
                    }

                    break;

                case LONG_START_SEQUENCE:

                    pFreq_2_Time_data_2 =
                        &pFreq_2_Time_data_1[ HALF_LONG_WINDOW];

                    pLong_Window_1 = &Long_Window_fxp[wnd_shape_prev_bk][0];
                    pLong_Window_2 = &pLong_Window_1[ HALF_LONG_WINDOW];

                    pOverlap_and_Add_Buffer_1 = &Time_data[0];
                    pOverlap_and_Add_Buffer_2 = &Time_data[HALF_LONG_WINDOW];

                    pOutput_buffer   = Output_buffer;
                    pOutput_buffer_2 = pOutput_buffer + HALF_LONG_WINDOW;

                    shift = exp + 15 - SCALING;

                    for (i = HALF_LONG_WINDOW; i != 0; i--)
                    {

                        Int16 win1, win2;
                        Int16  dat1, dat2;
                        Int32  test1, test2;

                        dat1   = *(pFreq_2_Time_data_1++);
                        win1   = *(pLong_Window_1++);
                        test1  = *(pOverlap_and_Add_Buffer_1++);

                        dat2  =  *(pFreq_2_Time_data_2++);
                        win2  = *(pLong_Window_2++);
                        test2 = *(pOverlap_and_Add_Buffer_2++);

                        limiter(*(pOutput_buffer++), (test1 + (fxp_mul_16_by_16(dat1, win1) >> shift)));

                        limiter(*(pOutput_buffer_2++), (test2 + (fxp_mul_16_by_16(dat2, win2) >> shift)));

                    }

                    pOverlap_and_Add_Buffer_1 = &Time_data[0];
                    pFreq_2_Time_data_1       = &pFreqInfo[LONG_WINDOW];

                    exp -= SCALING;

                    if (exp >= 0)
                    {

                        for (i = (W_L_START_1 - LONG_WINDOW) >> 1; i != 0; i--)
                        {
                            *(pOverlap_and_Add_Buffer_1++) =
                                *(pFreq_2_Time_data_1++) >> exp;
                            *(pOverlap_and_Add_Buffer_1++) =
                                *(pFreq_2_Time_data_1++) >> exp;

                        }

                    }
                    else if (exp < 0)
                    {

                        Int shift = -exp;
                        for (i = (W_L_START_1 - LONG_WINDOW) >> 1; i != 0 ; i--)
                        {
                            Int32 temp2 = ((Int32) * (pFreq_2_Time_data_1++)) << shift;
                            *(pOverlap_and_Add_Buffer_1++) = temp2;
                            temp2 = ((Int32) * (pFreq_2_Time_data_1++)) << shift;
                            *(pOverlap_and_Add_Buffer_1++) = temp2;
                        }

                    }
                    else
                    {

                        for (i = (W_L_START_1 - LONG_WINDOW) >> 1; i != 0; i--)
                        {
                            *(pOverlap_and_Add_Buffer_1++) =
                                *(pFreq_2_Time_data_1++);
                            *(pOverlap_and_Add_Buffer_1++) =
                                *(pFreq_2_Time_data_1++);

                        }

                    }

                    pFreq_2_Time_data_1  = &pFreqInfo[W_L_START_1];
                    pFreq_2_Time_data_2  =
                        &pFreq_2_Time_data_1[HALF_SHORT_WINDOW];

                    pShort_Window_1   =
                        &Short_Window_fxp[wnd_shape_this_bk][SHORT_WINDOW_m_1];

                    pShort_Window_2   = pShort_Window_1 - HALF_SHORT_WINDOW;

                    pOverlap_and_Add_Buffer_2 = pOverlap_and_Add_Buffer_1 +
                                                HALF_SHORT_WINDOW;

                    for (i = HALF_SHORT_WINDOW; i != 0; i--)
                    {
                        Int16 win1, win2;
                        Int16  dat1, dat2;
                        Int32  temp2;
                        dat1  = (*pFreq_2_Time_data_1++);
                        dat2  = (*pFreq_2_Time_data_2++);
                        win1  = *(pShort_Window_1--);
                        win2  = *(pShort_Window_2--);

                        temp   =   fxp_mul_16_by_16(dat1, win1) >> shift;
                        *(pOverlap_and_Add_Buffer_1++) = temp;

                        temp2 =   fxp_mul_16_by_16(dat2, win2) >> shift;
                        *(pOverlap_and_Add_Buffer_2++) = temp2;

                    }

                    pOverlap_and_Add_Buffer_1 += HALF_SHORT_WINDOW;

                    pv_memset(
                        pOverlap_and_Add_Buffer_1,
                        0,
                        (LONG_BLOCK1 - W_L_START_2)
                        *sizeof(*pOverlap_and_Add_Buffer_1));

                    break;

                case LONG_STOP_SEQUENCE:

                    pOverlap_and_Add_Buffer_1 = &Time_data[ W_L_STOP_2];

                    pOutput_buffer         = &Output_buffer[W_L_STOP_2];

                    pFreq_2_Time_data_1      = &pFreqInfo[W_L_STOP_2];

                    exp -= SCALING;

                    if (exp > 0)
                    {
                        Int16 tmp1 = (*(pFreq_2_Time_data_1++) >> exp);
                        temp = *(pOverlap_and_Add_Buffer_1++);

                        for (i = (LONG_WINDOW - W_L_STOP_2); i != 0; i--)
                        {
                            limiter(*(pOutput_buffer++), (temp + tmp1));

                            tmp1 = *(pFreq_2_Time_data_1++) >> exp;
                            temp = *(pOverlap_and_Add_Buffer_1++);

                        }
                    }
                    else if (exp < 0)
                    {
                        shift = -exp;
                        Int32 temp1 = ((Int32) * (pFreq_2_Time_data_1++)) << shift;
                        temp = *(pOverlap_and_Add_Buffer_1++);

                        for (i = (LONG_WINDOW - W_L_STOP_2); i != 0; i--)
                        {
                            limiter(*(pOutput_buffer++), (temp + temp1));

                            temp1 = ((Int32) * (pFreq_2_Time_data_1++)) << shift;
                            temp = *(pOverlap_and_Add_Buffer_1++);

                        }
                    }
                    else
                    {
                        Int16 tmp1 = *(pFreq_2_Time_data_1++);
                        temp = *(pOverlap_and_Add_Buffer_1++);

                        for (i = (LONG_WINDOW - W_L_STOP_2); i != 0; i--)
                        {
                            limiter(*(pOutput_buffer++), (temp + tmp1));

                            tmp1 = *(pFreq_2_Time_data_1++);
                            temp = *(pOverlap_and_Add_Buffer_1++);

                        }
                    }

                    pShort_Window_1 = &Short_Window_fxp[wnd_shape_prev_bk][0];
                    pShort_Window_2 = &pShort_Window_1[HALF_SHORT_WINDOW];

                    pFreq_2_Time_data_1 = &pFreqInfo[W_L_STOP_1];
                    pFreq_2_Time_data_2 =
                        &pFreq_2_Time_data_1[HALF_SHORT_WINDOW];

                    pOverlap_and_Add_Buffer_1 = &Time_data[ W_L_STOP_1];
                    pOverlap_and_Add_Buffer_2 = pOverlap_and_Add_Buffer_1
                                                + HALF_SHORT_WINDOW;

                    pOutput_buffer   = &Output_buffer[W_L_STOP_1];
                    pOutput_buffer_2 = pOutput_buffer + HALF_SHORT_WINDOW;

                    exp += SCALING;

                    shift = exp + 15 - SCALING;

                    for (i = HALF_SHORT_WINDOW; i != 0; i--)
                    {
                        Int16 win1;
                        Int16  dat1;

                        dat1 = *(pFreq_2_Time_data_1++);
                        win1 = *(pShort_Window_1++);
                        temp = *(pOverlap_and_Add_Buffer_1++);

                        test  = fxp_mul_16_by_16(dat1, win1);

                        limiter(*(pOutput_buffer++), (temp + (test >> shift)));

                        dat1 = *(pFreq_2_Time_data_2++);
                        win1 = *(pShort_Window_2++);
                        temp = *(pOverlap_and_Add_Buffer_2++);
                        test =  fxp_mul_16_by_16(dat1, win1);
                        limiter(*(pOutput_buffer_2++), (temp + (test >> shift)));

                    }

                    pFreq_2_Time_data_2 = &pFreqInfo[LONG_WINDOW];

                    pOverlap_and_Add_Buffer_1 = Time_data;

                    pOutput_buffer = Output_buffer;

                    pLong_Window_2   =
                        &Long_Window_fxp[wnd_shape_this_bk][LONG_WINDOW_m_1];

                    for (i = W_L_STOP_1; i != 0; i--)
                    {
                        Int16 win1;
                        Int16 dat1;

                        win1 = *(pLong_Window_2--);
                        dat1 = *pFreq_2_Time_data_2++;

                        limiter(*(pOutput_buffer++), *(pOverlap_and_Add_Buffer_1));

                        temp = fxp_mul_16_by_16(dat1, win1) >> shift;
                        *(pOverlap_and_Add_Buffer_1++) = temp ;

                    }

                    for (i = (LONG_WINDOW - W_L_STOP_1); i != 0; i--)
                    {
                        temp = fxp_mul_16_by_16(*pFreq_2_Time_data_2++, *(pLong_Window_2--)) >> shift;
                        *(pOverlap_and_Add_Buffer_1++) = temp ;
                    }

                    break;

            }

        }

        else
        {

            pOverlap_and_Add_Buffer_1 = &Time_data[0];

            pOutput_buffer = Output_buffer;

            temp  = (*pOverlap_and_Add_Buffer_1++);

            for (i = LONG_WINDOW; i != 0; i--)
            {

                limiter(*(pOutput_buffer++), temp);

                temp = (*pOverlap_and_Add_Buffer_1++);

            }

            pv_memset(Time_data, 0, LONG_WINDOW*sizeof(Time_data[0]));

        }

    }
    else
    {

        Int32 *pScrath_mem;
        Int32 *pScrath_mem_entry;
        Int32  *pFrequency_data = Frequency_data;

        Int32 * pOverlap_and_Add_Buffer_1;
        Int32 * pOverlap_and_Add_Buffer_2;
        Int32 * pOverlap_and_Add_Buffer_1x;
        Int32 * pOverlap_and_Add_Buffer_2x;

        pOverlap_and_Add_Buffer_1  = &pFrequency_data[
                                         LONG_WINDOW + 3*SHORT_WINDOW + HALF_SHORT_WINDOW];

        pv_memset(
            pOverlap_and_Add_Buffer_1,
            0,
            SHORT_WINDOW*sizeof(*pOverlap_and_Add_Buffer_1));

        for (wnd = NUM_SHORT_WINDOWS - 1; wnd >= NUM_SHORT_WINDOWS / 2 + 1; wnd--)
        {

            pFreqInfo = (Int16 *) & pFrequency_data[ wnd*SHORT_WINDOW];

            exp = imdct_fxp(
                      (Int32 *)pFreqInfo,
                      freq_2_time_buffer,
                      SHORT_BLOCK1,
                      Q_format,
                      abs_max_per_window[wnd]);

            pOverlap_and_Add_Buffer_1 =
                &pFrequency_data[ W_L_STOP_1 + SHORT_WINDOW*wnd];

            pOverlap_and_Add_Buffer_2 =
                pOverlap_and_Add_Buffer_1 + SHORT_WINDOW;

            if (exp < 16)
            {

                pFreq_2_Time_data_1 = &pFreqInfo[0];
                pFreq_2_Time_data_2 = &pFreqInfo[SHORT_WINDOW];

                pShort_Window_1 = &Short_Window_fxp[wnd_shape_this_bk][0];

                pShort_Window_2   =
                    &Short_Window_fxp[wnd_shape_this_bk][SHORT_WINDOW_m_1];

                shift = exp + 15 - SCALING;

                for (i = SHORT_WINDOW; i != 0; i--)
                {
                    Int16 win1, win2;
                    Int16  dat1, dat2;

                    dat2 = *(pFreq_2_Time_data_2++);
                    win2 = *(pShort_Window_2--);
                    temp = *pOverlap_and_Add_Buffer_2;
                    dat1 = *(pFreq_2_Time_data_1++);
                    win1 = *(pShort_Window_1++);

                    *(pOverlap_and_Add_Buffer_2++) =  temp + (fxp_mul_16_by_16(dat2, win2) >> shift);

                    *(pOverlap_and_Add_Buffer_1++)  =  fxp_mul_16_by_16(dat1, win1) >> shift;

                }

            }
            else
            {
                pv_memset(
                    pOverlap_and_Add_Buffer_1,
                    0,
                    SHORT_WINDOW*sizeof(*pOverlap_and_Add_Buffer_1));
            }

        }

        wnd = NUM_SHORT_WINDOWS / 2;

        pFreqInfo = (Int16 *) & pFrequency_data[ wnd*SHORT_WINDOW];

        pScrath_mem = &pFrequency_data[ 2*LONG_WINDOW - HALF_SHORT_WINDOW];

        pOverlap_and_Add_Buffer_1 = &pFrequency_data[ LONG_WINDOW];

        pOverlap_and_Add_Buffer_2 = pOverlap_and_Add_Buffer_1
                                    + HALF_SHORT_WINDOW;

        exp = imdct_fxp(
                  (Int32 *)pFreqInfo,
                  freq_2_time_buffer,
                  SHORT_BLOCK1,
                  Q_format,
                  abs_max_per_window[wnd]);

        if (exp < 16)
        {

            pFreq_2_Time_data_1 = &pFreqInfo[0];
            pFreq_2_Time_data_2 = &pFreqInfo[SHORT_WINDOW];

            pShort_Window_1 = &Short_Window_fxp[wnd_shape_this_bk][0];

            pShort_Window_2 =
                &Short_Window_fxp[wnd_shape_this_bk][SHORT_WINDOW_m_1];

            shift = exp + 15 - SCALING;
            {
                Int16 win1;
                Int16  dat1;

                dat1 = *(pFreq_2_Time_data_1++);
                win1 = *(pShort_Window_1++);
                for (i = HALF_SHORT_WINDOW; i != 0; i--)
                {
                    *(pScrath_mem++)  =  fxp_mul_16_by_16(dat1, win1) >> (shift);
                    dat1 = *(pFreq_2_Time_data_1++);
                    win1 = *(pShort_Window_1++);
                }

                for (i = HALF_SHORT_WINDOW; i != 0; i--)
                {
                    *(pOverlap_and_Add_Buffer_1++)  =  fxp_mul_16_by_16(dat1, win1) >> shift;

                    dat1 = *(pFreq_2_Time_data_1++);
                    win1 = *(pShort_Window_1++);
                }

                temp = *pOverlap_and_Add_Buffer_2;
                dat1 = *(pFreq_2_Time_data_2++);
                win1 = *(pShort_Window_2--);

                for (i = SHORT_WINDOW; i != 0; i--)
                {
                    *(pOverlap_and_Add_Buffer_2++)  =  temp + (fxp_mul_16_by_16(dat1, win1) >> shift);

                    temp = *pOverlap_and_Add_Buffer_2;
                    dat1 = *(pFreq_2_Time_data_2++);
                    win1 = *(pShort_Window_2--);
                }
            }

        }
        else
        {
            pv_memset(
                pScrath_mem,
                0,
                HALF_SHORT_WINDOW*sizeof(*pScrath_mem));

            pv_memset(
                pOverlap_and_Add_Buffer_1,
                0,
                HALF_SHORT_WINDOW*sizeof(*pOverlap_and_Add_Buffer_1));
        }

        wnd = NUM_SHORT_WINDOWS / 2 - 1;

        pFreqInfo = (Int16 *) & pFrequency_data[ wnd*SHORT_WINDOW];

        pScrath_mem_entry =
            &pFrequency_data[2*LONG_WINDOW - HALF_SHORT_WINDOW - SHORT_WINDOW];
        pScrath_mem = pScrath_mem_entry;

        pOverlap_and_Add_Buffer_1 = &pFrequency_data[ LONG_WINDOW];

        pOutput_buffer_2 = &Output_buffer[LONG_WINDOW - HALF_SHORT_WINDOW];
        pOutput_buffer   = pOutput_buffer_2;

        pOverlap_and_Add_Buffer_1x = &Time_data[W_L_STOP_1 + SHORT_WINDOW*(wnd+1)];

        exp = imdct_fxp(
                  (Int32 *)pFreqInfo,
                  freq_2_time_buffer,
                  SHORT_BLOCK1,
                  Q_format,
                  abs_max_per_window[wnd]);

        if (exp < 16)
        {

            pFreq_2_Time_data_1 = &pFreqInfo[0];
            pFreq_2_Time_data_2 = &pFreqInfo[SHORT_WINDOW];

            pShort_Window_1 = &Short_Window_fxp[wnd_shape_this_bk][0];

            pShort_Window_2 =
                &Short_Window_fxp[wnd_shape_this_bk][SHORT_WINDOW_m_1];

            shift = exp + 15 - SCALING;

            Int16 win1;
            Int16  dat1;

            dat1 = *(pFreq_2_Time_data_1++);
            win1 = *(pShort_Window_1++);
            for (i = SHORT_WINDOW; i != 0; i--)
            {
                *(pScrath_mem++)  =  fxp_mul_16_by_16(dat1, win1) >> shift;
                dat1 = *(pFreq_2_Time_data_1++);
                win1 = *(pShort_Window_1++);
            }

            dat1 = *(pFreq_2_Time_data_2++);
            win1 = *(pShort_Window_2--);

            for (i = HALF_SHORT_WINDOW; i != 0; i--)
            {
                test = fxp_mul_16_by_16(dat1, win1) >> shift;

                temp =  *(pScrath_mem++) + test;

                test = *(pOverlap_and_Add_Buffer_1x++);

                limiter(*(pOutput_buffer++), (temp + test));

                dat1 = *(pFreq_2_Time_data_2++);
                win1 = *(pShort_Window_2--);

            }

            for (i = HALF_SHORT_WINDOW; i != 0; i--)
            {
                temp = fxp_mul_16_by_16(dat1, win1) >> (shift);

                *(pOverlap_and_Add_Buffer_1++) += temp;

                dat1 = *(pFreq_2_Time_data_2++);
                win1 = *(pShort_Window_2--);
            }

        }
        else
        {

            pv_memset(
                pScrath_mem,
                0,
                SHORT_WINDOW*sizeof(*pScrath_mem));

            pScrath_mem += SHORT_WINDOW;

            temp = *(pScrath_mem++);
            for (i = HALF_SHORT_WINDOW; i != 0; i--)
            {
                limiter(*(pOutput_buffer++), temp);
                temp = *(pScrath_mem++);

            }
        }

        for (wnd = NUM_SHORT_WINDOWS / 2 - 2; wnd >= 0; wnd--)
        {

            pOutput_buffer_2 -= SHORT_WINDOW;
            pOutput_buffer = pOutput_buffer_2;

            pScrath_mem = pScrath_mem_entry;

            pOverlap_and_Add_Buffer_2x =
                &Time_data[W_L_STOP_1 + SHORT_WINDOW*(wnd+1)];

            pFreqInfo = (Int16 *) & pFrequency_data[ wnd*SHORT_WINDOW];

            exp = imdct_fxp(
                      (Int32 *)pFreqInfo,
                      freq_2_time_buffer,
                      SHORT_BLOCK1,
                      Q_format,
                      abs_max_per_window[wnd]);

            if (exp < 16)
            {

                pFreq_2_Time_data_1 = &pFreqInfo[0];
                pFreq_2_Time_data_2 = &pFreqInfo[SHORT_WINDOW];

                pShort_Window_1 = &Short_Window_fxp[wnd_shape_this_bk][0];

                if (wnd == 0)
                {
                    pShort_Window_1 =
                        &Short_Window_fxp[wnd_shape_prev_bk][0];
                }

                pShort_Window_2   =
                    &Short_Window_fxp[wnd_shape_this_bk][SHORT_WINDOW_m_1];

                shift = exp + 15 - SCALING;

                Int16 dat1 = *(pFreq_2_Time_data_2++);
                Int16 win1 = *(pShort_Window_2--);

                temp  =  *(pScrath_mem);
                for (i = SHORT_WINDOW; i != 0; i--)
                {
                    test  =  fxp_mul_16_by_16(dat1, win1) >> shift;

                    temp += test;

                    dat1 = *(pFreq_2_Time_data_1++);
                    win1 = *(pShort_Window_1++);

                    limiter(*(pOutput_buffer++), (temp + *(pOverlap_and_Add_Buffer_2x++)));

                    *(pScrath_mem++) = fxp_mul_16_by_16(dat1, win1) >> shift;
                    dat1 = *(pFreq_2_Time_data_2++);
                    win1 = *(pShort_Window_2--);
                    temp  =  *(pScrath_mem);

                }

            }
            else
            {
                test  = *(pScrath_mem);
                temp  = *(pOverlap_and_Add_Buffer_2x++);

                for (i = SHORT_WINDOW; i != 0; i--)
                {
                    limiter(*(pOutput_buffer++), (temp + test));

                    *(pScrath_mem++) = 0;
                    test  =  *(pScrath_mem);
                    temp  = *(pOverlap_and_Add_Buffer_2x++);

                }
            }

        }

        pOverlap_and_Add_Buffer_2x =  &Time_data[W_L_STOP_1];

        pScrath_mem = pScrath_mem_entry;

        pOutput_buffer_2 -= SHORT_WINDOW;
        pOutput_buffer = pOutput_buffer_2;

        test  = *(pScrath_mem++);
        temp  = *(pOverlap_and_Add_Buffer_2x++);

        for (i = SHORT_WINDOW; i != 0; i--)
        {
            limiter(*(pOutput_buffer++), (temp + test));

            test  = *(pScrath_mem++);
            temp  = *(pOverlap_and_Add_Buffer_2x++);

        }

        pOverlap_and_Add_Buffer_1x = Time_data;

        pOutput_buffer = Output_buffer;

        temp = *(pOverlap_and_Add_Buffer_1x++);

        for (i = W_L_STOP_1; i != 0; i--)
        {
            limiter(*(pOutput_buffer++), temp);

            temp = *(pOverlap_and_Add_Buffer_1x++);
        }

        pOverlap_and_Add_Buffer_1x = &Time_data[0];

        pOverlap_and_Add_Buffer_2 = &pFrequency_data[LONG_WINDOW];

        for (i = 0; i < W_L_STOP_2; i++)
        {
            temp = *(pOverlap_and_Add_Buffer_2++);
            *(pOverlap_and_Add_Buffer_1x++) = temp;
        }

        pv_memset(
            pOverlap_and_Add_Buffer_1x,
            0,
            W_L_STOP_1*sizeof(*pOverlap_and_Add_Buffer_1x));

    }

}

#endif

void trans4m_freq_2_time_fxp_2(
    Int32               Frequency_data[],
    Int32               Time_data[],
    WINDOW_SEQUENCE     wnd_seq,
    Int                 wnd_shape_prev_bk,
    Int                 wnd_shape_this_bk,
    Int                 Q_format,
    Int32               abs_max_per_window[],
    Int32               freq_2_time_buffer[],
    Int16               *Interleaved_output)

{

    Int exp;
    Int shift;

    Int  i;
    Int  wnd;
#if !(defined( PV_ARM_GCC_V5)||(PV_ARM_V5))
    Int32 z;
#endif
    Int16 *pFreqInfo;
    Int32 temp;
    Int32 test;

    Int16 *pFreq_2_Time_data_1;
    Int16 *pFreq_2_Time_data_2;

    const Int16 *pLong_Window_1;
    const Int16 *pLong_Window_2;
    const Int16 *pShort_Window_1;
    const Int16 *pShort_Window_2;

    Int32 *pOverlap_and_Add_Buffer_1;
    Int32 *pOverlap_and_Add_Buffer_2;

    Int16  *pInterleaved_output;
    Int16  *pInterleaved_output_2;

    const Int16 * Long_Window_fxp[NUM_WINDOW_SHAPES];
    const Int16 * Short_Window_fxp[NUM_WINDOW_SHAPES];

    Long_Window_fxp[0] = Long_Window_sine_fxp;
    Long_Window_fxp[1] = Long_Window_KBD_fxp;
    Short_Window_fxp[0] = Short_Window_sine_fxp;
    Short_Window_fxp[1] = Short_Window_KBD_fxp;

    if (wnd_seq != EIGHT_SHORT_SEQUENCE)
    {

        pFreqInfo = (Int16 *)Frequency_data;

        exp = imdct_fxp(
                  (Int32 *)pFreqInfo,
                  freq_2_time_buffer,
                  LONG_BLOCK1,
                  Q_format,
                  abs_max_per_window[0]);

        if (exp < 16)
        {

            pFreq_2_Time_data_1 = pFreqInfo;

            switch (wnd_seq)
            {

                case ONLY_LONG_SEQUENCE:
                default:

                {
                    pOverlap_and_Add_Buffer_1 = Time_data;

                    pInterleaved_output = Interleaved_output;

                    {

                        const Int16 *pLong_Window_2 = &Long_Window_fxp[wnd_shape_this_bk][LONG_WINDOW_m_1];

                        Int32 * pFreq2T   = (Int32 *)pFreqInfo;
                        Int32 * pFreq2T_2 = &pFreq2T[HALF_LONG_WINDOW];
                        Int32 * win = (Int32 *) & Long_Window_fxp[wnd_shape_prev_bk][0];

                        Int shift = exp + 15 - SCALING;

                        for (i = HALF_LONG_WINDOW; i != 0; i--)
                        {
                            Int16 win1, win2;
                            Int32  temp2, test2;

                            Int32  winx;

                            temp2 = *(pFreq2T++);
                            winx = *(win++);

                            test  = *(pOverlap_and_Add_Buffer_1++);
                            test2 = *(pOverlap_and_Add_Buffer_1--);
                            temp  =   fxp_mul_16_by_16bb(temp2, winx) >> shift;
                            temp2 =   fxp_mul_16_by_16tt(temp2, winx) >> shift;

                            limiter(*(pInterleaved_output), (temp + test));

                            limiter(*(pInterleaved_output + 2), (temp2 + test2));
                            pInterleaved_output += 4;

                            temp2 = *(pFreq2T_2++);

                            win1  = *(pLong_Window_2--);
                            win2  = *(pLong_Window_2--);
                            temp  = fxp_mul_16_by_16bb(temp2, win1) >> shift;
                            test2 = fxp_mul_16_by_16tb(temp2, win2) >> shift;

                            *(pOverlap_and_Add_Buffer_1++) = temp;
                            *(pOverlap_and_Add_Buffer_1++) = test2;
                        }

                    }

                }

                break;

                case LONG_START_SEQUENCE:

                    pFreq_2_Time_data_2 =
                        &pFreq_2_Time_data_1[ HALF_LONG_WINDOW];

                    pLong_Window_1 = &Long_Window_fxp[wnd_shape_prev_bk][0];
                    pLong_Window_2 = &pLong_Window_1[ HALF_LONG_WINDOW];

                    pOverlap_and_Add_Buffer_1 = &Time_data[0];
                    pOverlap_and_Add_Buffer_2 = &Time_data[HALF_LONG_WINDOW];

                    pInterleaved_output   = Interleaved_output;
                    pInterleaved_output_2 = pInterleaved_output + (2 * HALF_LONG_WINDOW);

                    shift = exp + 15 - SCALING;

                    for (i = HALF_LONG_WINDOW; i != 0; i--)
                    {
                        Int16 win1, win2;
                        Int16  dat1, dat2;
                        Int32  test1, test2;

                        dat1   = *(pFreq_2_Time_data_1++);
                        win1   = *(pLong_Window_1++);
                        test1  = *(pOverlap_and_Add_Buffer_1++);

                        dat2  =  *(pFreq_2_Time_data_2++);
                        win2  = *(pLong_Window_2++);
                        test2 = *(pOverlap_and_Add_Buffer_2++);

                        limiter(*(pInterleaved_output), (test1 + (fxp_mul_16_by_16(dat1, win1) >> shift)));

                        pInterleaved_output   += 2;

                        limiter(*(pInterleaved_output_2), (test2 + (fxp_mul_16_by_16(dat2, win2) >> shift)));

                        pInterleaved_output_2    += 2;
                    }

                    pOverlap_and_Add_Buffer_1 = &Time_data[0];
                    pFreq_2_Time_data_1       = &pFreqInfo[LONG_WINDOW];

                    exp -= SCALING;

                    if (exp >= 0)
                    {

                        for (i = (W_L_START_1 - LONG_WINDOW) >> 1; i != 0; i--)
                        {
                            *(pOverlap_and_Add_Buffer_1++) =
                                *(pFreq_2_Time_data_1++) >> exp;
                            *(pOverlap_and_Add_Buffer_1++) =
                                *(pFreq_2_Time_data_1++) >> exp;

                        }

                    }
                    else if (exp < 0)
                    {

                        Int shift = -exp;
                        for (i = (W_L_START_1 - LONG_WINDOW) >> 1; i != 0 ; i--)
                        {
                            Int32 temp2 = ((Int32) * (pFreq_2_Time_data_1++)) << shift;
                            *(pOverlap_and_Add_Buffer_1++) = temp2;
                            temp2 = ((Int32) * (pFreq_2_Time_data_1++)) << shift;
                            *(pOverlap_and_Add_Buffer_1++) = temp2;
                        }

                    }
                    else
                    {

                        for (i = (W_L_START_1 - LONG_WINDOW) >> 1; i != 0; i--)
                        {
                            *(pOverlap_and_Add_Buffer_1++) =
                                *(pFreq_2_Time_data_1++);
                            *(pOverlap_and_Add_Buffer_1++) =
                                *(pFreq_2_Time_data_1++);

                        }

                    }

                    pFreq_2_Time_data_1  = &pFreqInfo[W_L_START_1];
                    pFreq_2_Time_data_2  =
                        &pFreq_2_Time_data_1[HALF_SHORT_WINDOW];

                    pShort_Window_1   =
                        &Short_Window_fxp[wnd_shape_this_bk][SHORT_WINDOW_m_1];

                    pShort_Window_2   = pShort_Window_1 - HALF_SHORT_WINDOW;

                    pOverlap_and_Add_Buffer_2 = pOverlap_and_Add_Buffer_1 +
                                                HALF_SHORT_WINDOW;

                    {
                        Int16 win1, win2;
                        Int16  dat1, dat2;
                        Int32  temp2;

                        for (i = HALF_SHORT_WINDOW; i != 0; i--)
                        {

                            dat1  = (*pFreq_2_Time_data_1++);
                            dat2  = (*pFreq_2_Time_data_2++);
                            win1  = *(pShort_Window_1--);
                            win2  = *(pShort_Window_2--);

                            temp   =   fxp_mul_16_by_16(dat1, win1) >> shift;
                            *(pOverlap_and_Add_Buffer_1++) = temp;

                            temp2 =   fxp_mul_16_by_16(dat2, win2) >> shift;
                            *(pOverlap_and_Add_Buffer_2++) = temp2;

                        }
                    }

                    pOverlap_and_Add_Buffer_1 += HALF_SHORT_WINDOW;

                    pv_memset(
                        pOverlap_and_Add_Buffer_1,
                        0,
                        (LONG_BLOCK1 - W_L_START_2)
                        *sizeof(*pOverlap_and_Add_Buffer_1));

                    break;

                case LONG_STOP_SEQUENCE:

                    pOverlap_and_Add_Buffer_1 = &Time_data[ W_L_STOP_2];

                    pInterleaved_output    = &Interleaved_output[2*W_L_STOP_2];

                    pFreq_2_Time_data_1      = &pFreqInfo[W_L_STOP_2];

                    exp -= SCALING;

                    if (exp > 0)
                    {
                        Int16 tmp1 = (*(pFreq_2_Time_data_1++) >> exp);
                        temp = *(pOverlap_and_Add_Buffer_1++);

                        for (i = (LONG_WINDOW - W_L_STOP_2); i != 0; i--)
                        {
                            limiter(*(pInterleaved_output), (temp + tmp1));

                            pInterleaved_output += 2;
                            tmp1 = *(pFreq_2_Time_data_1++) >> exp;
                            temp = *(pOverlap_and_Add_Buffer_1++);
                        }
                    }
                    else if (exp < 0)
                    {
                        shift = -exp;

                        Int32 temp1 = ((Int32) * (pFreq_2_Time_data_1++)) << shift;
                        temp = *(pOverlap_and_Add_Buffer_1++);

                        for (i = (LONG_WINDOW - W_L_STOP_2); i != 0; i--)
                        {
                            limiter(*(pInterleaved_output), (temp + temp1));

                            pInterleaved_output += 2;
                            temp1 = ((Int32) * (pFreq_2_Time_data_1++)) << shift;
                            temp = *(pOverlap_and_Add_Buffer_1++);
                        }
                    }
                    else
                    {
                        Int16 tmp1 = *(pFreq_2_Time_data_1++);
                        temp = *(pOverlap_and_Add_Buffer_1++);
                        for (i = (LONG_WINDOW - W_L_STOP_2); i != 0; i--)
                        {
                            limiter(*(pInterleaved_output), (temp + tmp1));

                            pInterleaved_output += 2;
                            tmp1 = *(pFreq_2_Time_data_1++);
                            temp = *(pOverlap_and_Add_Buffer_1++);
                        }
                    }

                    pShort_Window_1 = &Short_Window_fxp[wnd_shape_prev_bk][0];
                    pShort_Window_2 = &pShort_Window_1[HALF_SHORT_WINDOW];

                    pFreq_2_Time_data_1 = &pFreqInfo[W_L_STOP_1];
                    pFreq_2_Time_data_2 =
                        &pFreq_2_Time_data_1[HALF_SHORT_WINDOW];

                    pOverlap_and_Add_Buffer_1 = &Time_data[ W_L_STOP_1];
                    pOverlap_and_Add_Buffer_2 = pOverlap_and_Add_Buffer_1
                                                + HALF_SHORT_WINDOW;

                    pInterleaved_output   = &Interleaved_output[2*W_L_STOP_1];
                    pInterleaved_output_2 = pInterleaved_output + (2 * HALF_SHORT_WINDOW);

                    exp += SCALING;
                    shift = exp + 15 - SCALING;

                    for (i = HALF_SHORT_WINDOW; i != 0; i--)
                    {

                        Int16 win1;
                        Int16 dat1;

                        dat1 = *(pFreq_2_Time_data_1++);
                        win1 = *(pShort_Window_1++);
                        temp = *(pOverlap_and_Add_Buffer_1++);

                        test  = fxp_mul_16_by_16(dat1, win1);

                        limiter(*(pInterleaved_output), (temp + (test >> shift)));

                        pInterleaved_output += 2;

                        dat1 = *(pFreq_2_Time_data_2++);
                        win1 = *(pShort_Window_2++);
                        temp = *(pOverlap_and_Add_Buffer_2++);
                        test =  fxp_mul_16_by_16(dat1, win1);

                        limiter(*(pInterleaved_output_2), (temp + (test >> shift)));

                        pInterleaved_output_2 += 2;

                    }

                    pFreq_2_Time_data_2 = &pFreqInfo[LONG_WINDOW];

                    pOverlap_and_Add_Buffer_1 = Time_data;

                    pInterleaved_output = Interleaved_output;

                    pLong_Window_2   =
                        &Long_Window_fxp[wnd_shape_this_bk][LONG_WINDOW_m_1];

                    for (i = W_L_STOP_1; i != 0; i--)
                    {

                        Int16 win1;
                        Int16 dat1;

                        win1 = *(pLong_Window_2--);
                        dat1 = *pFreq_2_Time_data_2++;

                        limiter(*(pInterleaved_output), *(pOverlap_and_Add_Buffer_1));

                        pInterleaved_output += 2;

                        temp = fxp_mul_16_by_16(dat1, win1) >> shift;
                        *(pOverlap_and_Add_Buffer_1++) = temp ;

                    }

                    for (i = (LONG_WINDOW - W_L_STOP_1); i != 0; i--)
                    {

                        temp = fxp_mul_16_by_16(*pFreq_2_Time_data_2++, *(pLong_Window_2--)) >> shift;
                        *(pOverlap_and_Add_Buffer_1++) = temp ;

                    }

                    break;

            }

        }

        else
        {

            pOverlap_and_Add_Buffer_1 = &Time_data[0];

            pInterleaved_output = Interleaved_output;

            temp  = (*pOverlap_and_Add_Buffer_1++);
            for (i = LONG_WINDOW; i != 0; i--)
            {

                limiter(*(pInterleaved_output), temp);

                pInterleaved_output += 2;
                temp  = (*pOverlap_and_Add_Buffer_1++);
            }
            pv_memset(Time_data, 0, LONG_WINDOW*sizeof(Time_data[0]));
        }

    }
    else
    {

        Int32 *pScrath_mem;
        Int32 *pScrath_mem_entry;
        Int32  *pFrequency_data = Frequency_data;

        Int32 * pOverlap_and_Add_Buffer_1;
        Int32 * pOverlap_and_Add_Buffer_2;
        Int32 * pOverlap_and_Add_Buffer_1x;
        Int32 * pOverlap_and_Add_Buffer_2x;

        pOverlap_and_Add_Buffer_1  = &pFrequency_data[
                                         LONG_WINDOW + 3*SHORT_WINDOW + HALF_SHORT_WINDOW];

        pv_memset(
            pOverlap_and_Add_Buffer_1,
            0,
            SHORT_WINDOW*sizeof(*pOverlap_and_Add_Buffer_1));

        for (wnd = NUM_SHORT_WINDOWS - 1; wnd >= NUM_SHORT_WINDOWS / 2 + 1; wnd--)
        {

            pFreqInfo = (Int16 *) & pFrequency_data[ wnd*SHORT_WINDOW];

            exp = imdct_fxp(
                      (Int32 *)pFreqInfo,
                      freq_2_time_buffer,
                      SHORT_BLOCK1,
                      Q_format,
                      abs_max_per_window[wnd]);

            pOverlap_and_Add_Buffer_1 =
                &pFrequency_data[ W_L_STOP_1 + SHORT_WINDOW*wnd];

            pOverlap_and_Add_Buffer_2 =
                pOverlap_and_Add_Buffer_1 + SHORT_WINDOW;

            if (exp < 16)
            {

                pFreq_2_Time_data_1 = &pFreqInfo[0];
                pFreq_2_Time_data_2 = &pFreqInfo[SHORT_WINDOW];

                pShort_Window_1 = &Short_Window_fxp[wnd_shape_this_bk][0];

                pShort_Window_2   =
                    &Short_Window_fxp[wnd_shape_this_bk][SHORT_WINDOW_m_1];

                shift = exp + 15 - SCALING;

                for (i = SHORT_WINDOW; i != 0; i--)
                {
                    Int16 win1, win2;
                    Int16  dat1, dat2;

                    dat2 = *(pFreq_2_Time_data_2++);
                    win2 = *(pShort_Window_2--);
                    temp = *pOverlap_and_Add_Buffer_2;
                    dat1 = *(pFreq_2_Time_data_1++);
                    win1 = *(pShort_Window_1++);

                    *(pOverlap_and_Add_Buffer_2++) =  temp + (fxp_mul_16_by_16(dat2, win2) >> shift);

                    *(pOverlap_and_Add_Buffer_1++)  =  fxp_mul_16_by_16(dat1, win1) >> shift;

                }

            }
            else
            {
                pv_memset(
                    pOverlap_and_Add_Buffer_1,
                    0,
                    SHORT_WINDOW*sizeof(*pOverlap_and_Add_Buffer_1));
            }

        }

        wnd = NUM_SHORT_WINDOWS / 2;

        pFreqInfo = (Int16 *) & pFrequency_data[ wnd*SHORT_WINDOW];

        pScrath_mem = &pFrequency_data[ 2*LONG_WINDOW - HALF_SHORT_WINDOW];

        pOverlap_and_Add_Buffer_1 = &pFrequency_data[ LONG_WINDOW];

        pOverlap_and_Add_Buffer_2 = pOverlap_and_Add_Buffer_1
                                    + HALF_SHORT_WINDOW;

        exp = imdct_fxp(
                  (Int32 *)pFreqInfo,
                  freq_2_time_buffer,
                  SHORT_BLOCK1,
                  Q_format,
                  abs_max_per_window[wnd]);

        if (exp < 16)
        {

            pFreq_2_Time_data_1 = &pFreqInfo[0];
            pFreq_2_Time_data_2 = &pFreqInfo[SHORT_WINDOW];

            pShort_Window_1 = &Short_Window_fxp[wnd_shape_this_bk][0];

            pShort_Window_2 =
                &Short_Window_fxp[wnd_shape_this_bk][SHORT_WINDOW_m_1];

            shift = exp + 15 - SCALING;
            {
                Int16 win1;
                Int16  dat1;

                dat1 = *(pFreq_2_Time_data_1++);
                win1 = *(pShort_Window_1++);
                for (i = HALF_SHORT_WINDOW; i != 0; i--)
                {
                    *(pScrath_mem++)  =  fxp_mul_16_by_16(dat1, win1) >> shift;
                    dat1 = *(pFreq_2_Time_data_1++);
                    win1 = *(pShort_Window_1++);
                }

                for (i = HALF_SHORT_WINDOW; i != 0; i--)
                {
                    *(pOverlap_and_Add_Buffer_1++)  =  fxp_mul_16_by_16(dat1, win1) >> shift;

                    dat1 = *(pFreq_2_Time_data_1++);
                    win1 = *(pShort_Window_1++);
                }

                temp = *pOverlap_and_Add_Buffer_2;
                dat1 = *(pFreq_2_Time_data_2++);
                win1 = *(pShort_Window_2--);

                for (i = SHORT_WINDOW; i != 0; i--)
                {
                    *(pOverlap_and_Add_Buffer_2++)  =  temp + (fxp_mul_16_by_16(dat1, win1) >> shift);

                    temp = *pOverlap_and_Add_Buffer_2;
                    dat1 = *(pFreq_2_Time_data_2++);
                    win1 = *(pShort_Window_2--);
                }
            }

        }
        else
        {
            pv_memset(
                pScrath_mem,
                0,
                HALF_SHORT_WINDOW*sizeof(*pScrath_mem));

            pv_memset(
                pOverlap_and_Add_Buffer_1,
                0,
                HALF_SHORT_WINDOW*sizeof(*pOverlap_and_Add_Buffer_1));
        }

        wnd = NUM_SHORT_WINDOWS / 2 - 1;

        pFreqInfo = (Int16 *) & pFrequency_data[ wnd*SHORT_WINDOW];

        pScrath_mem_entry =
            &pFrequency_data[2*LONG_WINDOW - HALF_SHORT_WINDOW - SHORT_WINDOW];

        pScrath_mem = pScrath_mem_entry;

        pOverlap_and_Add_Buffer_1 = &pFrequency_data[ LONG_WINDOW];

        pInterleaved_output_2 = &Interleaved_output[2*(LONG_WINDOW - HALF_SHORT_WINDOW)];
        pInterleaved_output = pInterleaved_output_2;

        pOverlap_and_Add_Buffer_1x = &Time_data[W_L_STOP_1 + SHORT_WINDOW*(wnd+1)];

        exp = imdct_fxp(
                  (Int32 *)pFreqInfo,
                  freq_2_time_buffer,
                  SHORT_BLOCK1,
                  Q_format,
                  abs_max_per_window[wnd]);

        if (exp < 16)
        {

            pFreq_2_Time_data_1 = &pFreqInfo[0];
            pFreq_2_Time_data_2 = &pFreqInfo[SHORT_WINDOW];

            pShort_Window_1 = &Short_Window_fxp[wnd_shape_this_bk][0];

            pShort_Window_2 =
                &Short_Window_fxp[wnd_shape_this_bk][SHORT_WINDOW_m_1];

            shift = exp + 15 - SCALING;

            Int16 win1;
            Int16  dat1;

            dat1 = *(pFreq_2_Time_data_1++);
            win1 = *(pShort_Window_1++);
            for (i = SHORT_WINDOW; i != 0; i--)
            {
                *(pScrath_mem++)  =  fxp_mul_16_by_16(dat1, win1) >> shift;
                dat1 = *(pFreq_2_Time_data_1++);
                win1 = *(pShort_Window_1++);
            }

            dat1 = *(pFreq_2_Time_data_2++);
            win1 = *(pShort_Window_2--);

            for (i = HALF_SHORT_WINDOW; i != 0; i--)
            {
                test = fxp_mul_16_by_16(dat1, win1) >> shift;

                temp =  *(pScrath_mem++) + test;

                test = *(pOverlap_and_Add_Buffer_1x++);
                limiter(*(pInterleaved_output), (temp + test));

                pInterleaved_output += 2;
                dat1 = *(pFreq_2_Time_data_2++);
                win1 = *(pShort_Window_2--);

            }

            for (i = HALF_SHORT_WINDOW; i != 0; i--)
            {

                temp = fxp_mul_16_by_16(dat1, win1) >> shift;

                *(pOverlap_and_Add_Buffer_1++) += temp;

                dat1 = *(pFreq_2_Time_data_2++);
                win1 = *(pShort_Window_2--);
            }

        }
        else
        {

            pv_memset(
                pScrath_mem,
                0,
                SHORT_WINDOW*sizeof(*pScrath_mem));

            pScrath_mem += SHORT_WINDOW;

            temp = *(pScrath_mem++);
            for (i = HALF_SHORT_WINDOW; i != 0; i--)
            {
                limiter(*(pInterleaved_output), (temp));

                pInterleaved_output += 2;
                temp = *(pScrath_mem++);

            }
        }

        for (wnd = NUM_SHORT_WINDOWS / 2 - 2; wnd >= 0; wnd--)
        {

            pInterleaved_output_2 -= (SHORT_WINDOW * 2);
            pInterleaved_output = pInterleaved_output_2;

            pScrath_mem = pScrath_mem_entry;

            pOverlap_and_Add_Buffer_2x =
                &Time_data[W_L_STOP_1 + SHORT_WINDOW*(wnd+1)];

            pFreqInfo = (Int16 *) & pFrequency_data[ wnd*SHORT_WINDOW];

            exp = imdct_fxp(
                      (Int32 *)pFreqInfo,
                      freq_2_time_buffer,
                      SHORT_BLOCK1,
                      Q_format,
                      abs_max_per_window[wnd]);

            if (exp < 16)
            {

                pFreq_2_Time_data_1 = &pFreqInfo[0];
                pFreq_2_Time_data_2 = &pFreqInfo[SHORT_WINDOW];

                pShort_Window_1 = &Short_Window_fxp[wnd_shape_this_bk][0];

                if (wnd == 0)
                {
                    pShort_Window_1 =
                        &Short_Window_fxp[wnd_shape_prev_bk][0];
                }

                pShort_Window_2   =
                    &Short_Window_fxp[wnd_shape_this_bk][SHORT_WINDOW_m_1];

                shift = exp + 15 - SCALING;

                Int16 dat1 = *(pFreq_2_Time_data_2++);
                Int16 win1 = *(pShort_Window_2--);

                temp  =  *(pScrath_mem);
                for (i = SHORT_WINDOW; i != 0; i--)
                {
                    test  =  fxp_mul_16_by_16(dat1, win1) >> shift;

                    temp += test;
                    dat1 = *(pFreq_2_Time_data_1++);
                    win1 = *(pShort_Window_1++);

                    limiter(*(pInterleaved_output), (temp + *(pOverlap_and_Add_Buffer_2x++)));

                    pInterleaved_output += 2;

                    *(pScrath_mem++) = fxp_mul_16_by_16(dat1, win1) >> shift;
                    dat1 = *(pFreq_2_Time_data_2++);
                    win1 = *(pShort_Window_2--);
                    temp  =  *(pScrath_mem);

                }

            }
            else
            {
                test  = *(pScrath_mem);
                temp  = *(pOverlap_and_Add_Buffer_2x++);

                for (i = SHORT_WINDOW; i != 0; i--)
                {
                    limiter(*(pInterleaved_output), (temp + test));

                    pInterleaved_output += 2;

                    *(pScrath_mem++) = 0;
                    test  =  *(pScrath_mem);
                    temp  = *(pOverlap_and_Add_Buffer_2x++);
                }
            }

        }

        pOverlap_and_Add_Buffer_2x =  &Time_data[W_L_STOP_1];

        pScrath_mem = pScrath_mem_entry;

        pInterleaved_output_2 -= (SHORT_WINDOW * 2);
        pInterleaved_output    = pInterleaved_output_2;

        test  = *(pScrath_mem++);
        temp  = *(pOverlap_and_Add_Buffer_2x++);

        for (i = SHORT_WINDOW; i != 0; i--)
        {
            limiter(*(pInterleaved_output), (temp + test));

            pInterleaved_output += 2;
            test  = *(pScrath_mem++);
            temp  = *(pOverlap_and_Add_Buffer_2x++);

        }

        pOverlap_and_Add_Buffer_1x = Time_data;

        pInterleaved_output = Interleaved_output;

        temp = *(pOverlap_and_Add_Buffer_1x++);
        for (i = W_L_STOP_1; i != 0; i--)
        {
            limiter(*(pInterleaved_output), temp);

            pInterleaved_output += 2;
            temp = *(pOverlap_and_Add_Buffer_1x++);

        }

        pOverlap_and_Add_Buffer_1x = &Time_data[0];

        pOverlap_and_Add_Buffer_2 = &pFrequency_data[LONG_WINDOW];

        for (i = 0; i < W_L_STOP_2; i++)
        {
            temp = *(pOverlap_and_Add_Buffer_2++);
            *(pOverlap_and_Add_Buffer_1x++) = temp;
        }

        pv_memset(
            pOverlap_and_Add_Buffer_1x,
            0,
            W_L_STOP_1*sizeof(*pOverlap_and_Add_Buffer_1x));

    }

}

