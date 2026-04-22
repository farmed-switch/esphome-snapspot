

#include "pv_audio_type_defs.h"
#include "s_tns_frame_info.h"
#include "s_tnsfilt.h"
#include "s_frameinfo.h"
#include "tns_inv_filter.h"
#include "tns_ar_filter.h"
#include "apply_tns.h"

void apply_tns(
    Int32                  coef[],
    Int                    q_format[],
    const FrameInfo      * const pFrameInfo,
    TNS_frame_info * const pTNS_frame_info,
    const Bool                   inverse_flag,
    Int32                  scratch_Int_buffer[])
{
    Int num_tns_bands;
    Int num_TNS_coef;

    Int f;

    Int tempInt;
    Int tempInt2;

    Int sfb_per_win;
    Int sfbWidth;

    Int coef_per_win;
    Int min_q;
    Int win;

    Int32 *pCoef = coef;
    Int32 *pTempCoef;

    Int   *pStartQformat = q_format;

    Int   *pQformat;
    Int32 *pLpcCoef;

    Int sfb_offset;

    const Int16 *pWinSfbTop;

    TNSfilt *pFilt;

    coef_per_win = pFrameInfo->coef_per_win[0];
    sfb_per_win  = pFrameInfo->sfb_per_win[0];

    win = 0;

    pLpcCoef = pTNS_frame_info->lpc_coef;

    pFilt = pTNS_frame_info->filt;

    do
    {
        for (f = pTNS_frame_info->n_filt[win]; f > 0; f--)
        {

            tempInt = pFilt->order;

            if (tempInt > 0)
            {

                num_TNS_coef = (pFilt->stop_coef - pFilt->start_coef);

                if (num_TNS_coef > 0)
                {
                    if (inverse_flag != FALSE)
                    {
                        tns_inv_filter(
                            &(pCoef[pFilt->start_coef]),
                            num_TNS_coef,
                            pFilt->direction,
                            pLpcCoef,
                            pFilt->q_lpc,
                            pFilt->order,
                            scratch_Int_buffer);
                    }
                    else
                    {
                        num_tns_bands = (pFilt->stop_band - pFilt->start_band);

                        pQformat =
                            &(pStartQformat[pFilt->stop_band]);

                        min_q = INT16_MAX;

                        for (tempInt = num_tns_bands; tempInt > 0; tempInt--)
                        {
                            tempInt2 = *(--pQformat);

                            if (tempInt2 < min_q)
                            {
                                min_q = tempInt2;
                            }
                        }

                        tempInt = pFilt->start_band;

                        tempInt--;

                        if (tempInt >= 0)
                        {
                            pWinSfbTop =
                                &(pFrameInfo->win_sfb_top[win][tempInt]);

                            sfb_offset = *(pWinSfbTop++);
                        }
                        else
                        {
                            pWinSfbTop = pFrameInfo->win_sfb_top[win];
                            sfb_offset = 0;
                        }

                        pTempCoef = pCoef + pFilt->start_coef;

                        for (tempInt = num_tns_bands; tempInt > 0; tempInt--)
                        {
                            sfbWidth  = *(pWinSfbTop++) - sfb_offset;

                            sfb_offset += sfbWidth;

                            tempInt2 = *(pQformat++) - min_q;

                            if (tempInt2 > 31)
                            {
                                tempInt2 = 31;
                            }

                            for (sfbWidth >>= 2; sfbWidth > 0; sfbWidth--)
                            {
                                *(pTempCoef++) >>= tempInt2;
                                *(pTempCoef++) >>= tempInt2;
                                *(pTempCoef++) >>= tempInt2;
                                *(pTempCoef++) >>= tempInt2;
                            }

                        }

                        tempInt2 =
                            tns_ar_filter(
                                &(pCoef[pFilt->start_coef]),
                                num_TNS_coef,
                                pFilt->direction,
                                pLpcCoef,
                                pFilt->q_lpc,
                                pFilt->order);

                        min_q -= tempInt2;

                        for (tempInt = num_tns_bands; tempInt > 0; tempInt--)
                        {
                            *(--pQformat) = min_q;
                        }

                    }

                }

                pLpcCoef += pFilt->order;

            }

            pFilt++;

        }

        pCoef += coef_per_win;
        pStartQformat += sfb_per_win;

        win++;

    }
    while (win < pFrameInfo->num_win);

    return;

}
