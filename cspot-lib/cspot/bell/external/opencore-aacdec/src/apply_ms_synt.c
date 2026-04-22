

#include "pv_audio_type_defs.h"
#include "apply_ms_synt.h"
#include "e_huffmanconst.h"
#include "ms_synt.h"

void apply_ms_synt(
    const FrameInfo * const pFrameInfo,
    const Int        group[],
    const Bool       mask_map[],
    const Int        codebook_map[],
    Int32      coefLeft[],
    Int32      coefRight[],
    Int        q_formatLeft[MAXBANDS],
    Int        q_formatRight[MAXBANDS])

{

    Int32   *pCoefRight;

    Int32   *pCoefLeft;

    Int     tot_sfb;
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

    const Int16 *pBand;
    const Int   *pCodebookMap  = codebook_map;
    const Int   *pGroup        = group;
    const Bool  *pMaskMap      = mask_map;

    Bool mask_enabled;

    pCoefRight = coefRight;
    pCoefLeft = coefLeft;

    window_start = 0;
    tot_sfb = 0;

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

            if (codebook < NOISE_HCB)
            {
                if (mask_enabled != FALSE)
                {
                    band_length = band_stop - band_start;

                    ms_synt(
                        wins_in_group,
                        coef_per_win,
                        sfb_per_win,
                        band_length,
                        &(pCoefLeft[band_start]),
                        &(pCoefRight[band_start]),
                        &(q_formatLeft[tot_sfb]),
                        &(q_formatRight[tot_sfb]));
                }
            }
            band_start = band_stop;

            tot_sfb++;

        }

        pCoefRight += coef_per_win * wins_in_group;
        pCoefLeft  += coef_per_win * wins_in_group--;

        tot_sfb += sfb_per_win * wins_in_group;

        window_start = partition;

    }
    while (partition < pFrameInfo->num_win);

    return;

}

