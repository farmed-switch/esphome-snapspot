

#include "pv_audio_type_defs.h"
#include "intensity_right.h"
#include "e_huffmanconst.h"

#include "fxp_mul32.h"
#include "aac_mem_funcs.h"

const Int16 intensity_factor[4] =
{
    32767,
    27554,
    23170,
    19484
};

void intensity_right(
    const Int   scalefactor,
    const Int   coef_per_win,
    const Int   sfb_per_win,
    const Int   wins_in_group,
    const Int   band_length,
    const Int   codebook,
    const Bool  ms_used,
    const Int   q_formatLeft[],
    Int   q_formatRight[],
    const Int32 coefLeft[],
    Int32 coefRight[])

{
    const Int32 *pCoefLeft  = coefLeft;
    Int32 *pCoefRight = coefRight;

    const Int *pQformatLeft  = q_formatLeft;
    Int *pQformatRight = q_formatRight;

    Int   multiplier;
    Int   scf_div_4;
    Int   nextWinPtrUpdate;
    Int   win_indx;
    Int   tempInt;

    multiplier  = (codebook & 0x1);

    multiplier ^= ms_used;

    multiplier <<= 1;

    multiplier--;

    multiplier *= intensity_factor[scalefactor & 0x3];

    scf_div_4 = (scalefactor >> 2);

    nextWinPtrUpdate = (coef_per_win - band_length);

    for (win_indx = wins_in_group; win_indx > 0; win_indx--)
    {

        *(pQformatRight) = scf_div_4 + *(pQformatLeft);

        if (multiplier == 32767)
        {
            Int32 tempInt2 = *(pCoefLeft++);
            Int32 tempInt22 = *(pCoefLeft++);

            for (tempInt = band_length >> 1; tempInt > 0; tempInt--)
            {
                *(pCoefRight++) = tempInt2;
                *(pCoefRight++) = tempInt22;
                tempInt2 = *(pCoefLeft++);
                tempInt22 = *(pCoefLeft++);
            }

        }
        else
        {

            Int32 tempInt2 = *(pCoefLeft++);
            Int32 tempInt22 = *(pCoefLeft++);
            for (tempInt = band_length >> 1; tempInt > 0; tempInt--)
            {
                *(pCoefRight++) = fxp_mul32_by_16(tempInt2, multiplier) << 1;
                *(pCoefRight++) = fxp_mul32_by_16(tempInt22, multiplier) << 1;
                tempInt2 = *(pCoefLeft++);
                tempInt22 = *(pCoefLeft++);
            }
        }

        pCoefRight += nextWinPtrUpdate;
        pCoefLeft  += (nextWinPtrUpdate - 2);

        pQformatRight += sfb_per_win;
        pQformatLeft  += sfb_per_win;

    }

}
