

#include "pv_audio_type_defs.h"
#include "ms_synt.h"
#include "fxp_mul32.h"
#include "aac_mem_funcs.h"
#include "window_block_fxp.h"

void ms_synt(
    const Int   wins_in_group,
    const Int   coef_per_win,
    const Int   num_bands,
    const Int   band_length,
    Int32 coefLeft[],
    Int32 coefRight[],
    Int q_formatLeft[],
    Int q_formatRight[])
{

    Int32 *pCoefL = coefLeft;
    Int32 *pCoefR = coefRight;
    Int start_indx = 0;
    Int win_indx;
    Int i;

    if (band_length < 0 || band_length > LONG_WINDOW)
    {
        return;
    }

    Int nextWinPtrUpdate = (coef_per_win - band_length);

    for (win_indx = wins_in_group; win_indx > 0; win_indx--)
    {

        if (q_formatRight[start_indx] < 31)
        {
            Int tempInt = q_formatLeft[start_indx] - q_formatRight[start_indx];

            if (tempInt > 0)
            {

                Int shift_left_chan  = 1 + tempInt;

                q_formatLeft[start_indx] = --(q_formatRight[start_indx]);

                Int32 temp_left  = *(pCoefL) >> shift_left_chan;
                Int32 temp_right = *(pCoefR) >> 1;

                for (i = band_length; i != 0; i--)
                {
                    *(pCoefL++) = temp_left + temp_right;
                    *(pCoefR++) = temp_left - temp_right;
                    temp_left  = *(pCoefL) >> shift_left_chan;
                    temp_right = *(pCoefR) >> 1;

                }

            }
            else
            {

                Int shift_right_chan = 1 - tempInt;

                q_formatRight[start_indx] = --(q_formatLeft[start_indx]);

                Int32 temp_left  = *(pCoefL) >> 1;
                Int32 temp_right = *(pCoefR) >> shift_right_chan;

                for (i = band_length; i != 0; i--)
                {
                    *(pCoefL++) = temp_left + temp_right;
                    *(pCoefR++) = temp_left - temp_right;

                    temp_left  = *(pCoefL) >> 1;
                    temp_right = *(pCoefR) >> shift_right_chan;

                }
            }

        }
        else
        {

            q_formatRight[start_indx] = (q_formatLeft[start_indx]);

            pv_memcpy(pCoefR, pCoefL, band_length*sizeof(*pCoefR));
            pCoefR += band_length;
            pCoefL += band_length;
        }

        pCoefL += nextWinPtrUpdate;
        pCoefR += nextWinPtrUpdate;

        start_indx += num_bands;

    }

    return;

}
