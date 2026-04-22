

#include "pv_audio_type_defs.h"
#include "lt_decode.h"
#include "ltp_common_internal.h"
#include "window_block_fxp.h"
#include "e_window_sequence.h"
#include "s_lt_pred_status.h"
#include "s_bits.h"
#include "ibstream.h"

void lt_decode(
    const WINDOW_SEQUENCE  win_type,
    BITS            *pInputStream,
    const Int              max_sfb,
    LT_PRED_STATUS  *pLt_pred)
{
    Int wnd_num;
    Int k;
    Int last_band;
    Int prev_subblock;
    Int prev_subblock_nonzero;
    Int temp_reg;

    Bool *pWinPredictionUsed = pLt_pred->win_prediction_used;
    Bool *pSfbPredictionUsed = pLt_pred->sfb_prediction_used;
    Int  *pTempPtr;
    Int  *pDelay = pLt_pred->delay;

    pDelay[0] = (Int) get17_n_lessbits(
                    LEN_LTP_LAG,
                    pInputStream);

    pLt_pred->weight_index  = (Int) get9_n_lessbits(
                                  LEN_LTP_COEF,
                                  pInputStream);

    last_band = max_sfb;

    if (win_type != EIGHT_SHORT_SEQUENCE)
    {

        if (last_band > MAX_LT_PRED_LONG_SFB)
        {
            last_band = MAX_LT_PRED_LONG_SFB;
        }

        for (k = last_band; k > 0; k--)
        {
            *(pSfbPredictionUsed++) = (Int) get1bits(pInputStream);
        }

        for (k = (max_sfb - last_band); k > 0; k--)
        {
            *(pSfbPredictionUsed++) = FALSE;
        }
    }
    else
    {

        if (last_band > MAX_LT_PRED_SHORT_SFB)
        {
            last_band = MAX_LT_PRED_SHORT_SFB;
        }

        prev_subblock = pDelay[0];

        pTempPtr = &pSfbPredictionUsed[0];

        wnd_num = NUM_SHORT_WINDOWS;

        prev_subblock_nonzero = prev_subblock;
        prev_subblock += LTP_LAG_OFFSET;

        do
        {

            wnd_num--;

            temp_reg = (Int) get1bits(pInputStream);

            *(pWinPredictionUsed++) = temp_reg;

            if (temp_reg != FALSE)
            {
                *(pDelay++) = prev_subblock_nonzero;

                for (k = last_band; k > 0; k--)
                {
                    *(pTempPtr++) = TRUE;
                }
                for (k = (max_sfb - last_band); k > 0; k--)
                {
                    *(pTempPtr++) = FALSE;
                }
                break;

            }
            else
            {
                pDelay++;
                pTempPtr += max_sfb;
            }

        }
        while (wnd_num > 0);

        while (wnd_num > 0)
        {
            temp_reg = (Int) get1bits(pInputStream);

            *(pWinPredictionUsed++) = temp_reg;

            if (temp_reg != FALSE)
            {
                temp_reg = (Int) get1bits(pInputStream);
                if (temp_reg != 0)
                {
                    temp_reg  = (Int) get9_n_lessbits(
                                    LEN_LTP_SHORT_LAG,
                                    pInputStream);

                    *(pDelay++) = prev_subblock - temp_reg;
                }
                else
                {
                    *(pDelay++) = prev_subblock_nonzero;
                }
                for (k = last_band; k > 0; k--)
                {
                    *(pTempPtr++) = TRUE;
                }
                for (k = (max_sfb - last_band); k > 0; k--)
                {
                    *(pTempPtr++) = FALSE;
                }

            }
            else
            {
                pDelay++;
                pTempPtr += max_sfb;
            }

            wnd_num--;

        }

    }

}
