

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "SigProc_FIX.h"

opus_int32 silk_lin2log(
    const opus_int32            inLin
)
{
    opus_int32 lz, frac_Q7;

    silk_CLZ_FRAC( inLin, &lz, &frac_Q7 );

    return silk_ADD_LSHIFT32( silk_SMLAWB( frac_Q7, silk_MUL( frac_Q7, 128 - frac_Q7 ), 179 ), 31 - lz, 7 );
}

