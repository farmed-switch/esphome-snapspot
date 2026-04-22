

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "SigProc_FIX.h"

void silk_LPC_fit(
    opus_int16                  *a_QOUT,
    opus_int32                    *a_QIN,
    const opus_int              QOUT,
    const opus_int              QIN,
    const opus_int              d
)
{
    opus_int    i, k, idx = 0;
    opus_int32    maxabs, absval, chirp_Q16;

    for( i = 0; i < 10; i++ ) {

        maxabs = 0;
        for( k = 0; k < d; k++ ) {
            absval = silk_abs( a_QIN[k] );
            if( absval > maxabs ) {
                maxabs = absval;
                idx    = k;
            }
        }
        maxabs = silk_RSHIFT_ROUND( maxabs, QIN - QOUT );

        if( maxabs > silk_int16_MAX ) {

            maxabs = silk_min( maxabs, 163838 );
            chirp_Q16 = SILK_FIX_CONST( 0.999, 16 ) - silk_DIV32( silk_LSHIFT( maxabs - silk_int16_MAX, 14 ),
                                        silk_RSHIFT32( silk_MUL( maxabs, idx + 1), 2 ) );
            silk_bwexpander_32( a_QIN, d, chirp_Q16 );
        } else {
            break;
        }
    }

    if( i == 10 ) {

        for( k = 0; k < d; k++ ) {
            a_QOUT[ k ] = (opus_int16)silk_SAT16( silk_RSHIFT_ROUND( a_QIN[ k ], QIN - QOUT ) );
            a_QIN[ k ] = silk_LSHIFT( (opus_int32)a_QOUT[ k ], QIN - QOUT );
        }
    } else {
        for( k = 0; k < d; k++ ) {
            a_QOUT[ k ] = (opus_int16)silk_RSHIFT_ROUND( a_QIN[ k ], QIN - QOUT );
        }
    }
}
