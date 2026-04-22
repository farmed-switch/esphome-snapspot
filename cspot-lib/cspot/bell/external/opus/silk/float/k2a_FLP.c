

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "SigProc_FLP.h"

void silk_k2a_FLP(
    silk_float          *A,
    const silk_float    *rc,
    opus_int32          order
)
{
    opus_int   k, n;
    silk_float rck, tmp1, tmp2;

    for( k = 0; k < order; k++ ) {
        rck = rc[ k ];
        for( n = 0; n < (k + 1) >> 1; n++ ) {
            tmp1 = A[ n ];
            tmp2 = A[ k - n - 1 ];
            A[ n ]         = tmp1 + tmp2 * rck;
            A[ k - n - 1 ] = tmp2 + tmp1 * rck;
        }
        A[ k ] = -rck;
    }
}
