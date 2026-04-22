

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "SigProc_FLP.h"

silk_float silk_schur_FLP(
    silk_float          refl_coef[],
    const silk_float    auto_corr[],
    opus_int            order
)
{
    opus_int   k, n;
    double C[ SILK_MAX_ORDER_LPC + 1 ][ 2 ];
    double Ctmp1, Ctmp2, rc_tmp;

    celt_assert( order >= 0 && order <= SILK_MAX_ORDER_LPC );

    k = 0;
    do {
        C[ k ][ 0 ] = C[ k ][ 1 ] = auto_corr[ k ];
    } while( ++k <= order );

    for( k = 0; k < order; k++ ) {

        rc_tmp = -C[ k + 1 ][ 0 ] / silk_max_float( C[ 0 ][ 1 ], 1e-9f );

        refl_coef[ k ] = (silk_float)rc_tmp;

        for( n = 0; n < order - k; n++ ) {
            Ctmp1 = C[ n + k + 1 ][ 0 ];
            Ctmp2 = C[ n ][ 1 ];
            C[ n + k + 1 ][ 0 ] = Ctmp1 + Ctmp2 * rc_tmp;
            C[ n ][ 1 ]         = Ctmp2 + Ctmp1 * rc_tmp;
        }
    }

    return (silk_float)C[ 0 ][ 1 ];
}
