

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "SigProc_FIX.h"

void silk_k2a(
    opus_int32                  *A_Q24,
    const opus_int16            *rc_Q15,
    const opus_int32            order
)
{
    opus_int   k, n;
    opus_int32 rc, tmp1, tmp2;

    for( k = 0; k < order; k++ ) {
        rc = rc_Q15[ k ];
        for( n = 0; n < (k + 1) >> 1; n++ ) {
            tmp1 = A_Q24[ n ];
            tmp2 = A_Q24[ k - n - 1 ];
            A_Q24[ n ]         = silk_SMLAWB( tmp1, silk_LSHIFT( tmp2, 1 ), rc );
            A_Q24[ k - n - 1 ] = silk_SMLAWB( tmp2, silk_LSHIFT( tmp1, 1 ), rc );
        }
        A_Q24[ k ] = -silk_LSHIFT( (opus_int32)rc_Q15[ k ], 9 );
    }
}
