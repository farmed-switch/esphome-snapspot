

#include "pv_audio_type_defs.h"
#include "pns_left.h"
#include "e_huffmanconst.h"
#include "gen_rand_vector.h"

void pns_left(
    const FrameInfo *pFrameInfo,
    const Int        group[],
    const Int        codebook_map[],
    const Int        factors[],
    const Int        sfb_prediction_used[],
    const Bool       ltp_data_present,
    Int32      spectral_coef[],
    Int        q_format[],
    Int32     *pCurrentSeed)
{

    Int     tot_sfb;
    Int     start_indx;

    Int     sfb;
    Int     band_stop;

    const Int16  *pBand;

    const Int *pCodebookMap = &(codebook_map[0]);
    const Int *pGroup   = &(group[0]);
    const Int *pFactors = &(factors[0]);

    Int     tempInt;
    Int32  *pWindow_Coef;

    Int32   *spec;

    Int partition;
    Int win_indx;

    tot_sfb = 0;

    spec = spectral_coef;

    win_indx = 0;
    partition = 0;
    do
    {
        Int num_bands = pFrameInfo->sfb_per_win[partition];
        pBand = pFrameInfo->win_sfb_top[partition];

        partition = *pGroup++;

        do
        {
            Int band_start = 0;
            for (sfb = 0; sfb < num_bands; sfb++)
            {
                band_stop = pBand[sfb];

                Int band_length =  band_stop - band_start;
                if (pCodebookMap[sfb] == NOISE_HCB)
                {

                    tempInt = sfb_prediction_used[tot_sfb] & ltp_data_present;

                    if (tempInt == FALSE)
                    {

                        pWindow_Coef = spec + band_start;

                        tempInt = pFactors[sfb];

                        start_indx = tot_sfb++;

                        q_format[start_indx] = gen_rand_vector(pWindow_Coef,
                                                               band_length,
                                                               pCurrentSeed,
                                                               tempInt);

                    }

                }
                else
                {
                    tot_sfb ++;
                }

                band_start = band_stop;

            }

            spec += pFrameInfo->coef_per_win[win_indx++];
            pFactors += num_bands;

        }
        while (win_indx < partition);

        pCodebookMap += pFrameInfo->sfb_per_win[win_indx-1];

    }
    while (partition < pFrameInfo->num_win);

    return;

}
