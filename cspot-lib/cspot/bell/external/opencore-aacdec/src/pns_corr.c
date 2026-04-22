

#include "pv_audio_type_defs.h"
#include "pns_corr.h"

const UInt hcb2_scale_mod_4[4] =
{
    32768,
    38968,
    46341,
    55109
};

void pns_corr(
    const Int scale,
    const Int coef_per_win,
    const Int sfb_per_win,
    const Int wins_in_group,
    const Int band_length,
    const Int q_formatLeft,
    Int q_formatRight[],
    const Int32 coefLeft[],
    Int32 coefRight[])
{
    Int tempInt;
    Int nextWinPtrUpdate;

    Int q_format;

    Int start_indx;
    Int win_indx;

    const Int32   *pCoefLeft;
    Int32   *pCoefRight;

    UInt multiplier;

    q_format = q_formatLeft - (scale >> 2);

    q_format--;

    multiplier = hcb2_scale_mod_4[scale & 0x3];

    pCoefLeft  = coefLeft;
    pCoefRight = coefRight;

    nextWinPtrUpdate = (coef_per_win - band_length);

    start_indx = 0;

    for (win_indx = wins_in_group; win_indx > 0; win_indx--)
    {

        q_formatRight[start_indx] = q_format;

        start_indx += sfb_per_win;

        for (tempInt = band_length; tempInt > 0; tempInt--)
        {
            *(pCoefRight++) = (Int32)(*(pCoefLeft++) >> 16) * multiplier;
        }

        pCoefRight += nextWinPtrUpdate;
        pCoefLeft  += nextWinPtrUpdate;

    }

    return;

}
