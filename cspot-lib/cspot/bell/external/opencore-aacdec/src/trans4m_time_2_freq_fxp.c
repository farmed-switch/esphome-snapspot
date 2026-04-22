

#include "pv_audio_type_defs.h"
#include "aac_mem_funcs.h"
#include "window_block_fxp.h"
#include "mdct_fxp.h"
#include "long_term_prediction.h"
#include    "fxp_mul32.h"

void trans4m_time_2_freq_fxp(
    Int32   Time2Freq_data[],
    WINDOW_SEQUENCE wnd_seq,
    Int     wnd_shape_prev_bk,
    Int     wnd_shape_this_bk,
    Int     *pQ_format,
    Int32   mem_4_in_place_FFT[])
{

    Int  i;

    Int32   *pAux_temp_1;
    Int32   *pAux_temp_2;
    Int32   *pAux_temp;

    const   Int16 *pLong_Window_1;
    const   Int16 *pLong_Window_2;
    const   Int16 *pShort_Window_1;
    const   Int16 *pShort_Window_2;
    Int     shift = *pQ_format - 1;

    const Int16 * Long_Window_fxp[NUM_WINDOW_SHAPES];
    const Int16 * Short_Window_fxp[NUM_WINDOW_SHAPES];

    Long_Window_fxp[0] = Long_Window_sine_fxp;
    Long_Window_fxp[1] = Long_Window_KBD_fxp;
    Short_Window_fxp[0] = Short_Window_sine_fxp;
    Short_Window_fxp[1] = Short_Window_KBD_fxp;

    if (wnd_seq != EIGHT_SHORT_SEQUENCE)
    {

        pAux_temp = Time2Freq_data;

        *pQ_format = LTP_Q_FORMAT - *pQ_format;

        pAux_temp_1 = pAux_temp;

        switch (wnd_seq)
        {

            case LONG_START_SEQUENCE:

                pAux_temp_2 = &pAux_temp_1[HALF_LONG_WINDOW];

                pLong_Window_1 = &Long_Window_fxp[wnd_shape_prev_bk][0];
                pLong_Window_2 = &pLong_Window_1[ HALF_LONG_WINDOW];

                for (i = HALF_LONG_WINDOW; i > 0; i--)
                {

                    *pAux_temp_1 = fxp_mul32_by_16((*pAux_temp_1), *pLong_Window_1++) >> shift;
                    pAux_temp_1++;
                    *pAux_temp_2 = fxp_mul32_by_16((*pAux_temp_2), *pLong_Window_2++) >> shift;
                    pAux_temp_2++;

                }

                pAux_temp_1 = &pAux_temp[LONG_WINDOW];
                if (shift)
                {
                    for (i = (W_L_START_1 - LONG_WINDOW) >> 1; i != 0; i--)
                    {
                        *(pAux_temp_1++) >>= shift;
                        *(pAux_temp_1++) >>= shift;
                    }
                }

                pAux_temp_1 = &pAux_temp[W_L_START_1];
                pAux_temp_2 = &pAux_temp_1[HALF_SHORT_WINDOW];

                pShort_Window_1   =
                    &Short_Window_fxp[wnd_shape_this_bk][SHORT_WINDOW_m_1];

                pShort_Window_2   = pShort_Window_1 - HALF_SHORT_WINDOW;

                for (i = HALF_SHORT_WINDOW; i > 0; i--)
                {

                    *pAux_temp_1 = fxp_mul32_by_16((*pAux_temp_1), *pShort_Window_1--) >> shift;
                    pAux_temp_1++;
                    *pAux_temp_2 = fxp_mul32_by_16((*pAux_temp_2), *pShort_Window_2--) >> shift;
                    pAux_temp_2++;

                }

                pAux_temp_1 = &pAux_temp[W_L_START_2];

                pv_memset(
                    pAux_temp_1,
                    0,
                    (LONG_BLOCK1 - W_L_START_2)*sizeof(*pAux_temp_1));

                break;

            case LONG_STOP_SEQUENCE:

                pv_memset(
                    pAux_temp_1,
                    0,
                    (W_L_STOP_1)*sizeof(*pAux_temp_1));

                pShort_Window_1   = &Short_Window_fxp[wnd_shape_prev_bk][0];
                pShort_Window_2   = &pShort_Window_1[HALF_SHORT_WINDOW];

                pAux_temp_1      = &pAux_temp_1[W_L_STOP_1];
                pAux_temp_2      = pAux_temp_1 + HALF_SHORT_WINDOW;

                for (i = HALF_SHORT_WINDOW; i > 0; i--)
                {

                    *pAux_temp_1 = fxp_mul32_by_16((*pAux_temp_1), *pShort_Window_1++) >> shift;
                    pAux_temp_1++;
                    *pAux_temp_2 = fxp_mul32_by_16((*pAux_temp_2), *pShort_Window_2++) >> shift;
                    pAux_temp_2++;

                }

                pAux_temp_1 = &pAux_temp[W_L_STOP_2];

                if (shift)
                {
                    for (i = ((LONG_WINDOW - W_L_STOP_2) >> 1); i != 0; i--)
                    {
                        *(pAux_temp_1++) >>= shift;
                        *(pAux_temp_1++) >>= shift;
                    }
                }

                pAux_temp_1 = &pAux_temp[LONG_WINDOW];
                pAux_temp_2 =  pAux_temp_1 + HALF_LONG_WINDOW;

                pLong_Window_1 =
                    &Long_Window_fxp[wnd_shape_this_bk][LONG_WINDOW_m_1];

                pLong_Window_2   = &pLong_Window_1[-HALF_LONG_WINDOW];

                for (i = HALF_LONG_WINDOW; i > 0; i--)
                {
                    *pAux_temp_1 = fxp_mul32_by_16((*pAux_temp_1), *pLong_Window_1--) >> shift;
                    pAux_temp_1++;
                    *pAux_temp_2 = fxp_mul32_by_16((*pAux_temp_2), *pLong_Window_2--) >> shift;
                    pAux_temp_2++;

                }

                break;

            case ONLY_LONG_SEQUENCE:
            default:

                pAux_temp_2 = &pAux_temp[LONG_WINDOW];

                pLong_Window_1 = &Long_Window_fxp[wnd_shape_prev_bk][0];

                pLong_Window_2 =
                    &Long_Window_fxp[wnd_shape_this_bk][LONG_WINDOW_m_1];

                for (i = LONG_WINDOW; i > 0; i--)
                {

                    *pAux_temp_1 = fxp_mul32_by_16((*pAux_temp_1), *pLong_Window_1++) >> shift;
                    pAux_temp_1++;
                    *pAux_temp_2 = fxp_mul32_by_16((*pAux_temp_2), *pLong_Window_2--) >> shift;
                    pAux_temp_2++;
                }

                break;

        }

        *pQ_format += mdct_fxp(
                          pAux_temp,
                          mem_4_in_place_FFT,
                          LONG_BLOCK1);

    }

}
