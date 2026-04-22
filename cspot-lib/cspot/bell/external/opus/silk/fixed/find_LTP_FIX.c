

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main_FIX.h"
#include "tuning_parameters.h"

void silk_find_LTP_FIX(
    opus_int32                      XXLTP_Q17[ MAX_NB_SUBFR * LTP_ORDER * LTP_ORDER ],
    opus_int32                      xXLTP_Q17[ MAX_NB_SUBFR * LTP_ORDER ],
    const opus_int16                r_ptr[],
    const opus_int                  lag[ MAX_NB_SUBFR ],
    const opus_int                  subfr_length,
    const opus_int                  nb_subfr,
    int                             arch
)
{
    opus_int   i, k, extra_shifts;
    opus_int   xx_shifts, xX_shifts, XX_shifts;
    const opus_int16 *lag_ptr;
    opus_int32 *XXLTP_Q17_ptr, *xXLTP_Q17_ptr;
    opus_int32 xx, nrg, temp;

    xXLTP_Q17_ptr = xXLTP_Q17;
    XXLTP_Q17_ptr = XXLTP_Q17;
    for( k = 0; k < nb_subfr; k++ ) {
        lag_ptr = r_ptr - ( lag[ k ] + LTP_ORDER / 2 );

        silk_sum_sqr_shift( &xx, &xx_shifts, r_ptr, subfr_length + LTP_ORDER );
        silk_corrMatrix_FIX( lag_ptr, subfr_length, LTP_ORDER, XXLTP_Q17_ptr, &nrg, &XX_shifts, arch );
        extra_shifts = xx_shifts - XX_shifts;
        if( extra_shifts > 0 ) {

            xX_shifts = xx_shifts;
            for( i = 0; i < LTP_ORDER * LTP_ORDER; i++ ) {
                XXLTP_Q17_ptr[ i ] = silk_RSHIFT32( XXLTP_Q17_ptr[ i ], extra_shifts );
            }
            nrg = silk_RSHIFT32( nrg, extra_shifts );
        } else if( extra_shifts < 0 ) {

            xX_shifts = XX_shifts;
            xx = silk_RSHIFT32( xx, -extra_shifts );
        } else {
            xX_shifts = xx_shifts;
        }
        silk_corrVector_FIX( lag_ptr, r_ptr, subfr_length, LTP_ORDER, xXLTP_Q17_ptr, xX_shifts, arch );

        temp = silk_SMLAWB( 1, nrg, SILK_FIX_CONST( LTP_CORR_INV_MAX, 16 ) );
        temp = silk_max( temp, xx );
TIC(div)
#if 0
        for( i = 0; i < LTP_ORDER * LTP_ORDER; i++ ) {
            XXLTP_Q17_ptr[ i ] = silk_DIV32_varQ( XXLTP_Q17_ptr[ i ], temp, 17 );
        }
        for( i = 0; i < LTP_ORDER; i++ ) {
            xXLTP_Q17_ptr[ i ] = silk_DIV32_varQ( xXLTP_Q17_ptr[ i ], temp, 17 );
        }
#else
        for( i = 0; i < LTP_ORDER * LTP_ORDER; i++ ) {
            XXLTP_Q17_ptr[ i ] = (opus_int32)( silk_LSHIFT64( (opus_int64)XXLTP_Q17_ptr[ i ], 17 ) / temp );
        }
        for( i = 0; i < LTP_ORDER; i++ ) {
            xXLTP_Q17_ptr[ i ] = (opus_int32)( silk_LSHIFT64( (opus_int64)xXLTP_Q17_ptr[ i ], 17 ) / temp );
        }
#endif
TOC(div)
        r_ptr         += subfr_length;
        XXLTP_Q17_ptr += LTP_ORDER * LTP_ORDER;
        xXLTP_Q17_ptr += LTP_ORDER;
    }
}
