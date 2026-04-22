

#include "pv_audio_type_defs.h"
#include "e_window_sequence.h"
#include "long_term_synthesis.h"

void long_term_synthesis(
    WINDOW_SEQUENCE     win_seq,
    Int                 sfb_per_win,
    Int16               win_sfb_top[],
    Int                 win_prediction_used[],
    Int                 sfb_prediction_used[],
    Int32               current_frame[],
    Int                 q_format[],
    Int32               predicted_spectral[],
    Int                 pred_q_format,
    Int                 coef_per_win,
    Int                 short_window_num,
    Int                 reconstruct_sfb_num)
{

    Int sfb_offset;

    Int wnd;

    Int32 *pPredicted_spectral;

    Int32 *pPredicted_spectral_start;

    Int32 *pPredicted_offset;

    Int32 *pCurrent_frame;

    Int32 *pCurrent_frame_start;

    Int *pSfb_prediction_used;

    Int16 *pWinSfbTop;

    Int *pQ_format;
    Int *pQ_format_start;
    Int32   temp;

    Int i;
    Int j;

    Int quarter_sfb_width;
    Int num_sfb;
    Int shift_factor;

    UInt32  max;
    Int32   tmpInt32;

    Int tmpInt;
    Int adjusted_pred_q;
    Int pred_shift;

    pPredicted_spectral = &predicted_spectral[0];
    pPredicted_spectral_start = pPredicted_spectral;

    pSfb_prediction_used = &sfb_prediction_used[0];

    if (win_seq != EIGHT_SHORT_SEQUENCE)
    {

        sfb_offset = 0;

        pWinSfbTop = &win_sfb_top[0];

        pQ_format = &q_format[0];

        for (i = sfb_per_win; i > 0; i--)
        {

            if (*(pSfb_prediction_used++) != FALSE)
            {

                pPredicted_offset = pPredicted_spectral_start +
                                    sfb_offset;
                pCurrent_frame = &current_frame[sfb_offset];

                quarter_sfb_width = (*pWinSfbTop - sfb_offset) >> 2;

                max = 0;

                pPredicted_spectral = pPredicted_offset;

                for (j = (*pWinSfbTop - sfb_offset); j > 0 ; j--)
                {
                    tmpInt32 = *(pPredicted_spectral++);

                    max |= tmpInt32 ^(tmpInt32 >> 31);
                }

                if (max != 0)
                {

                    tmpInt = 0;

                    while (max < 0x40000000L)
                    {
                        max <<= 1;
                        tmpInt++;
                    }

                    adjusted_pred_q = pred_q_format + tmpInt;

                    pPredicted_spectral = pPredicted_offset;

                    shift_factor = *(pQ_format) - adjusted_pred_q;

                    if ((shift_factor >= 0) && (shift_factor < 31))
                    {
                        shift_factor = shift_factor + 1;
                        pred_shift = tmpInt - 1;

                        if (pred_shift >= 0)
                        {
                            for (j = quarter_sfb_width; j > 0 ; j--)
                            {
                                temp = *pCurrent_frame >> shift_factor;
                                *(pCurrent_frame++) = temp
                                                      + (*(pPredicted_spectral++) << pred_shift);
                                temp = *pCurrent_frame >> shift_factor;
                                *(pCurrent_frame++) = temp
                                                      + (*(pPredicted_spectral++) << pred_shift);
                                temp = *pCurrent_frame >> shift_factor;
                                *(pCurrent_frame++) = temp
                                                      + (*(pPredicted_spectral++) << pred_shift);
                                temp = *pCurrent_frame >> shift_factor;
                                *(pCurrent_frame++) = temp
                                                      + (*(pPredicted_spectral++) << pred_shift);
                            }
                        }
                        else
                        {
                            for (j = quarter_sfb_width; j > 0 ; j--)
                            {
                                temp = *pCurrent_frame >> shift_factor;
                                *(pCurrent_frame++) = temp
                                                      + (*(pPredicted_spectral++) >> 1);
                                temp = *pCurrent_frame >> shift_factor;
                                *(pCurrent_frame++) = temp
                                                      + (*(pPredicted_spectral++) >> 1);
                                temp = *pCurrent_frame >> shift_factor;
                                *(pCurrent_frame++) = temp
                                                      + (*(pPredicted_spectral++) >> 1);
                                temp = *pCurrent_frame >> shift_factor;
                                *(pCurrent_frame++) = temp
                                                      + (*(pPredicted_spectral++) >> 1);
                            }
                        }

                        *(pQ_format) = adjusted_pred_q  - 1;
                    }
                    else if (shift_factor >= 31)
                    {
                        for (j = quarter_sfb_width; j > 0 ; j--)
                        {
                            *(pCurrent_frame++) =
                                *(pPredicted_spectral++) << tmpInt;
                            *(pCurrent_frame++) =
                                *(pPredicted_spectral++) << tmpInt;
                            *(pCurrent_frame++) =
                                *(pPredicted_spectral++) << tmpInt;
                            *(pCurrent_frame++) =
                                *(pPredicted_spectral++) << tmpInt;
                        }

                        *(pQ_format) = adjusted_pred_q ;
                    }
                    else if ((shift_factor < 0) && (shift_factor > -31))
                    {
                        shift_factor = 1 - shift_factor;
                        pred_shift = tmpInt - shift_factor;

                        if (pred_shift >= 0)
                        {
                            for (j = quarter_sfb_width; j > 0 ; j--)
                            {
                                temp = *pCurrent_frame >> 1;
                                *(pCurrent_frame++) =  temp +
                                                       (*(pPredicted_spectral++) << pred_shift);
                                temp = *pCurrent_frame >> 1;
                                *(pCurrent_frame++) =  temp +
                                                       (*(pPredicted_spectral++) << pred_shift);
                                temp = *pCurrent_frame >> 1;
                                *(pCurrent_frame++) =  temp +
                                                       (*(pPredicted_spectral++) << pred_shift);
                                temp = *pCurrent_frame >> 1;
                                *(pCurrent_frame++) =  temp +
                                                       (*(pPredicted_spectral++) << pred_shift);
                            }
                        }
                        else
                        {
                            pred_shift = -pred_shift;

                            for (j = quarter_sfb_width; j > 0 ; j--)
                            {
                                temp = *pCurrent_frame >> 1;
                                *(pCurrent_frame++) =  temp +
                                                       (*(pPredicted_spectral++) >> pred_shift);
                                temp = *pCurrent_frame >> 1;
                                *(pCurrent_frame++) =  temp +
                                                       (*(pPredicted_spectral++) >> pred_shift);
                                temp = *pCurrent_frame >> 1;
                                *(pCurrent_frame++) =  temp +
                                                       (*(pPredicted_spectral++) >> pred_shift);
                                temp = *pCurrent_frame >> 1;
                                *(pCurrent_frame++) =  temp +
                                                       (*(pPredicted_spectral++) >> pred_shift);
                            }
                        }

                        (*pQ_format)--;
                    }

                }

            }

            sfb_offset = *(pWinSfbTop++);

            pQ_format++;

        }

    }

    else
    {

        pCurrent_frame_start = &current_frame[0];

        pQ_format_start = &q_format[0];

        num_sfb = sfb_per_win;

        for (wnd = 0; wnd < short_window_num; wnd++)
        {
            pWinSfbTop = &win_sfb_top[0];

            pQ_format = pQ_format_start;

            if (win_prediction_used[wnd] != FALSE)
            {

                sfb_offset = 0;

                for (i = reconstruct_sfb_num; i > 0; i--)
                {

                    pPredicted_offset = pPredicted_spectral_start +
                                        sfb_offset;
                    pCurrent_frame = pCurrent_frame_start + sfb_offset;

                    quarter_sfb_width = (*pWinSfbTop - sfb_offset) >> 2;

                    max = 0;
                    pPredicted_spectral = pPredicted_offset;

                    for (j = (*pWinSfbTop - sfb_offset); j > 0 ; j--)
                    {
                        tmpInt32 = *(pPredicted_spectral++);

                        max |= tmpInt32 ^(tmpInt32 >> 31);
                    }

                    if (max != 0)
                    {

                        tmpInt = 0;

                        while (max < 0x40000000L)
                        {
                            max <<= 1;
                            tmpInt++;
                        }

                        adjusted_pred_q = pred_q_format + tmpInt;

                        pPredicted_spectral = pPredicted_offset;

                        shift_factor = *(pQ_format) - adjusted_pred_q;

                        if ((shift_factor >= 0) && (shift_factor < 31))
                        {
                            shift_factor = shift_factor + 1;

                            pred_shift = tmpInt - 1;

                            if (pred_shift >= 0)
                            {
                                for (j = quarter_sfb_width; j > 0 ; j--)
                                {
                                    temp = *pCurrent_frame >> shift_factor;
                                    *(pCurrent_frame++) = temp
                                                          + (*(pPredicted_spectral++) << pred_shift);
                                    temp = *pCurrent_frame >> shift_factor;
                                    *(pCurrent_frame++) = temp
                                                          + (*(pPredicted_spectral++) << pred_shift);
                                    temp = *pCurrent_frame >> shift_factor;
                                    *(pCurrent_frame++) = temp
                                                          + (*(pPredicted_spectral++) << pred_shift);
                                    temp = *pCurrent_frame >> shift_factor;
                                    *(pCurrent_frame++) = temp
                                                          + (*(pPredicted_spectral++) << pred_shift);

                                }
                            }
                            else
                            {
                                for (j = quarter_sfb_width; j > 0 ; j--)
                                {
                                    temp = *pCurrent_frame >> shift_factor;
                                    *(pCurrent_frame++) = temp
                                                          + (*(pPredicted_spectral++) >> 1);
                                    temp = *pCurrent_frame >> shift_factor;
                                    *(pCurrent_frame++) = temp
                                                          + (*(pPredicted_spectral++) >> 1);
                                    temp = *pCurrent_frame >> shift_factor;
                                    *(pCurrent_frame++) = temp
                                                          + (*(pPredicted_spectral++) >> 1);
                                    temp = *pCurrent_frame >> shift_factor;
                                    *(pCurrent_frame++) = temp
                                                          + (*(pPredicted_spectral++) >> 1);
                                }
                            }

                            *(pQ_format) = adjusted_pred_q - 1;
                        }
                        else if (shift_factor >= 31)
                        {
                            for (j = quarter_sfb_width; j > 0 ; j--)
                            {
                                *(pCurrent_frame++) =
                                    *(pPredicted_spectral++) << tmpInt;
                                *(pCurrent_frame++) =
                                    *(pPredicted_spectral++) << tmpInt;
                                *(pCurrent_frame++) =
                                    *(pPredicted_spectral++) << tmpInt;
                                *(pCurrent_frame++) =
                                    *(pPredicted_spectral++) << tmpInt;
                            }

                            *(pQ_format) = adjusted_pred_q;
                        }
                        else if ((shift_factor < 0) && (shift_factor > -31))
                        {
                            shift_factor = 1 - shift_factor;

                            pred_shift = tmpInt - shift_factor;

                            if (pred_shift >= 0)
                            {
                                for (j = quarter_sfb_width; j > 0 ; j--)
                                {
                                    temp = *pCurrent_frame >> 1;
                                    *(pCurrent_frame++) =  temp +
                                                           (*(pPredicted_spectral++) << pred_shift);
                                    temp = *pCurrent_frame >> 1;
                                    *(pCurrent_frame++) =  temp +
                                                           (*(pPredicted_spectral++) << pred_shift);
                                    temp = *pCurrent_frame >> 1;
                                    *(pCurrent_frame++) =  temp +
                                                           (*(pPredicted_spectral++) << pred_shift);
                                    temp = *pCurrent_frame >> 1;
                                    *(pCurrent_frame++) =  temp +
                                                           (*(pPredicted_spectral++) << pred_shift);

                                }
                            }
                            else
                            {
                                pred_shift = -pred_shift;

                                for (j = quarter_sfb_width; j > 0 ; j--)
                                {
                                    temp = *pCurrent_frame >> 1;
                                    *(pCurrent_frame++) =  temp +
                                                           (*(pPredicted_spectral++) >> pred_shift);
                                    temp = *pCurrent_frame >> 1;
                                    *(pCurrent_frame++) =  temp +
                                                           (*(pPredicted_spectral++) >> pred_shift);
                                    temp = *pCurrent_frame >> 1;
                                    *(pCurrent_frame++) =  temp +
                                                           (*(pPredicted_spectral++) >> pred_shift);
                                    temp = *pCurrent_frame >> 1;
                                    *(pCurrent_frame++) =  temp +
                                                           (*(pPredicted_spectral++) >> pred_shift);
                                }
                            }

                            *(pQ_format) = *(pQ_format) - 1;
                        }

                    }

                    sfb_offset = *(pWinSfbTop++);

                    pQ_format++;

                }

            }

            pPredicted_spectral_start += coef_per_win;
            pCurrent_frame_start += coef_per_win;
            pQ_format_start += num_sfb;

        }

    }

    return;
}

