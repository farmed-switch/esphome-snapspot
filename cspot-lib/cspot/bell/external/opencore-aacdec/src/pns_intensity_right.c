

#include "pv_audio_type_defs.h"
#include "pns_intensity_right.h"
#include "e_huffmanconst.h"
#include "gen_rand_vector.h"
#include "intensity_right.h"
#include "pns_corr.h"

void pns_intensity_right(
    const Int        hasmask,
    const FrameInfo * const pFrameInfo,
    const Int        group[],
    const Bool       mask_map[],
    const Int        codebook_map[],
    const Int        factorsL[],
    const Int        factorsR[],
    Int        sfb_prediction_used[],
    const Bool       ltp_data_present,
    Int32      coefLeft[],
    Int32      coefRight[],
    Int        q_formatLeft[MAXBANDS],
    Int        q_formatRight[MAXBANDS],
    Int32 * const pCurrentSeed)
{

    Int32   *pCoefRight;
    Int32   *pWindow_CoefR;

    Int32   *pCoefLeft;

    Int     tot_sfb;
    Int     start_indx;
    Int     sfb;

    Int     band_length;
    Int     band_start;
    Int     band_stop;
    Int     coef_per_win;

    Int     codebook;
    Int     partition;
    Int     window_start;

    Int     sfb_per_win;
    Int     wins_in_group;
    Int     win_indx;

    const Int16 *pBand;
    const Int   *pFactorsLeft  = factorsL;
    const Int   *pFactorsRight = factorsR;
    const Int   *pCodebookMap  = codebook_map;
    const Int   *pGroup        = group;
    const Bool  *pMaskMap      = mask_map;

    Bool mask_enabled;

    pCoefRight = coefRight;
    pCoefLeft = coefLeft;

    window_start = 0;
    tot_sfb = 0;
    start_indx = 0;

    coef_per_win = pFrameInfo->coef_per_win[0];

    sfb_per_win = pFrameInfo->sfb_per_win[0];

    do
    {
        pBand     = pFrameInfo->win_sfb_top[window_start];

        partition = *(pGroup++);

        band_start = 0;

        wins_in_group = (partition - window_start);

        for (sfb = sfb_per_win; sfb > 0; sfb--)
        {

            band_stop = *(pBand++);

            codebook = *(pCodebookMap++);

            mask_enabled = *(pMaskMap++);

            band_length = band_stop - band_start;

            if (codebook == NOISE_HCB)
            {
                sfb_prediction_used[tot_sfb] &= ltp_data_present;

                if (sfb_prediction_used[tot_sfb] == FALSE)
                {

                    mask_enabled &= hasmask;

                    if (mask_enabled == FALSE)
                    {
                        pWindow_CoefR = &(pCoefRight[band_start]);

                        start_indx = tot_sfb;

                        for (win_indx = wins_in_group;
                                win_indx > 0;
                                win_indx--)
                        {

                            q_formatRight[start_indx] =
                                gen_rand_vector(
                                    pWindow_CoefR,
                                    band_length,
                                    pCurrentSeed,
                                    *(pFactorsRight));

                            pWindow_CoefR += coef_per_win;

                            start_indx += sfb_per_win;
                        }

                    }
                    else
                    {
                        pns_corr(
                            (*(pFactorsRight) -
                             *(pFactorsLeft)),
                            coef_per_win,
                            sfb_per_win,
                            wins_in_group,
                            band_length,
                            q_formatLeft[tot_sfb],
                            &(q_formatRight[tot_sfb]),
                            &(pCoefLeft[band_start]),
                            &(pCoefRight[band_start]));

                    }

                }

            }
            else if (codebook >= INTENSITY_HCB2)
            {

                mask_enabled &= hasmask;

                intensity_right(
                    *(pFactorsRight),
                    coef_per_win,
                    sfb_per_win,
                    wins_in_group,
                    band_length,
                    codebook,
                    mask_enabled,
                    &(q_formatLeft[tot_sfb]),
                    &(q_formatRight[tot_sfb]),
                    &(pCoefLeft[band_start]),
                    &(pCoefRight[band_start]));

            }

            band_start = band_stop;

            tot_sfb++;

            pFactorsLeft++;
            pFactorsRight++;

        }

        pCoefRight += coef_per_win * wins_in_group;
        pCoefLeft  += coef_per_win * wins_in_group--;

        tot_sfb += sfb_per_win * wins_in_group;

        pFactorsRight += sfb_per_win * wins_in_group;
        pFactorsLeft  += sfb_per_win * wins_in_group;

        window_start = partition;

    }
    while (partition < pFrameInfo->num_win);

    return;

}

