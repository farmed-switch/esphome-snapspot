

#include "pv_audio_type_defs.h"
#include "get_tns.h"
#include "s_mc_info.h"
#include "s_frameinfo.h"
#include "s_tnsfilt.h"
#include "s_tns_frame_info.h"
#include "s_bits.h"
#include "ibstream.h"
#include "e_window_sequence.h"
#include "e_progconfigconst.h"

#include "tns_decode_coef.h"

#define SCALE_FACTOR_BAND_OFFSET(x) ( ((x) > 0) ? pSFB_top[(x)-1] : 0 )
#define MINIMUM(x,y) ( ((x) < (y)) ? (x) : (y) )

const Int tns_max_bands_tbl_long_wndw[(1<<LEN_SAMP_IDX)] =
    {31,
     31,
     34,
     40,
     42,
     51,
     46,
     46,
     42,
     42,
     42,
     39,
     0,
     0,
     0,
     0
    };

const Int tns_max_bands_tbl_short_wndw[(1<<LEN_SAMP_IDX)] =
    {9,
     9,
     10,
     14,
     14,
     14,
     14,
     14,
     14,
     14,
     14,
     14,
     0,
     0,
     0,
     0
    };

void get_tns(
    const Int               max_bands,
    BITS            * const pInputStream,
    const WINDOW_SEQUENCE   wnd_seq,
    const FrameInfo * const pFrameInfo,
    const MC_Info   * const pMC_Info,
    TNS_frame_info  * const pTnsFrameInfo,
    Int32                   scratchTnsDecCoefMem[])
{

    const Int16 * const pSFB_top = pFrameInfo->win_sfb_top[0];

    Int f;
    Int t;
    Int win;
    UInt tempInt;

    Int num_filt_bits;
    Int num_order_bits;
    Int num_start_band_bits;

    Int top;
    Int res;
    Int res_index;
    Int compress;

    Int sfb_per_win;

    Int32 *pLpcCoef;
    Int32 *pStartLpcCoef;
    Int s_mask;
    Int n_mask;

    Int tns_bands;
    UInt max_order;
    Int coef_res;

    TNSfilt *pFilt;

    if (wnd_seq != EIGHT_SHORT_SEQUENCE)
    {
        num_filt_bits  = 2;
        num_order_bits = 5;
        num_start_band_bits = 6;

        tns_bands = tns_max_bands_tbl_long_wndw[pMC_Info->sampling_rate_idx];

        if (pMC_Info->sampling_rate_idx > 4)
        {
            max_order = 20;
        }
        else
        {
            max_order = 12;
        }
    }
    else
    {
        num_filt_bits  = 1;
        num_order_bits = 3;
        num_start_band_bits = 4;

        tns_bands = tns_max_bands_tbl_short_wndw[pMC_Info->sampling_rate_idx];

        max_order = 7;
    }

    if (max_bands < tns_bands)
    {
        tns_bands = max_bands;
    }

    sfb_per_win = pFrameInfo->sfb_per_win[0];

    win = 0;

    pLpcCoef = pTnsFrameInfo->lpc_coef;

    pFilt = pTnsFrameInfo->filt;

    do
    {
        tempInt = get9_n_lessbits(num_filt_bits,
                                  pInputStream);

        pTnsFrameInfo->n_filt[win] = tempInt;

        if (tempInt != 0)
        {

            res = get1bits(
                      pInputStream);

            coef_res = res++;

            top = sfb_per_win;

            for (f = pTnsFrameInfo->n_filt[win]; f > 0; f--)
            {
                tempInt = MINIMUM(top, tns_bands);

                pFilt->stop_coef = SCALE_FACTOR_BAND_OFFSET(tempInt);

                pFilt->stop_band = tempInt;

                top -= get9_n_lessbits(num_start_band_bits,
                                       pInputStream);

                tempInt = MINIMUM(top, tns_bands);

                pFilt->start_coef = SCALE_FACTOR_BAND_OFFSET(tempInt);

                pFilt->start_band = tempInt;

                tempInt = get9_n_lessbits(num_order_bits,
                                          pInputStream);

                pFilt->order = tempInt;

                if (tempInt != 0)
                {
                    if (tempInt > max_order)
                    {
                        pFilt->order = max_order;
                    }

                    tempInt = get1bits(pInputStream);

                    pFilt->direction = (-(Int)tempInt) | 0x1;

                    compress = get1bits(pInputStream);

                    res_index = res - compress;

                    s_mask =  2 << res_index;

                    res_index += 2;

                    pStartLpcCoef = pLpcCoef;

                    for (t = pFilt->order; t > 0; t--)
                    {

                        tempInt = get9_n_lessbits(res_index,
                                                  pInputStream);

                        n_mask  = -((Int)tempInt & s_mask);

                        *(pLpcCoef++) = tempInt | n_mask;
                    }

                    tempInt = pFilt->stop_coef - pFilt->start_coef;

                    if (tempInt > 0)
                    {
                        pFilt->q_lpc =
                            tns_decode_coef(
                                pFilt->order,
                                coef_res,
                                pStartLpcCoef,
                                scratchTnsDecCoefMem);
                    }

                }

                pFilt++;

            }

        }

        win++;

    }
    while (win < pFrameInfo->num_win);

    return;

}
