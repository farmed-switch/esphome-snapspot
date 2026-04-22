

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "SigProc_FIX.h"
#include "SigProc_FLP.h"
#include "define.h"

silk_float silk_LPC_inverse_pred_gain_FLP(
    const silk_float    *A,
    opus_int32          order
)
{
    opus_int   k, n;
    double     invGain, rc, rc_mult1, rc_mult2, tmp1, tmp2;
    silk_float Atmp[ SILK_MAX_ORDER_LPC ];

    silk_memcpy( Atmp, A, order * sizeof(silk_float) );

    invGain = 1.0;
    for( k = order - 1; k > 0; k-- ) {
        rc = -Atmp[ k ];
        rc_mult1 = 1.0f - rc * rc;
        invGain *= rc_mult1;
        if( invGain * MAX_PREDICTION_POWER_GAIN < 1.0f ) {
            return 0.0f;
        }
        rc_mult2 = 1.0f / rc_mult1;
        for( n = 0; n < (k + 1) >> 1; n++ ) {
            tmp1 = Atmp[ n ];
            tmp2 = Atmp[ k - n - 1 ];
            Atmp[ n ]         = (silk_float)( ( tmp1 - tmp2 * rc ) * rc_mult2 );
            Atmp[ k - n - 1 ] = (silk_float)( ( tmp2 - tmp1 * rc ) * rc_mult2 );
        }
    }
    rc = -Atmp[ 0 ];
    rc_mult1 = 1.0f - rc * rc;
    invGain *= rc_mult1;
    if( invGain * MAX_PREDICTION_POWER_GAIN < 1.0f ) {
        return 0.0f;
    }
    return (silk_float)invGain;
}
