

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main_FIX.h"

void silk_LTP_analysis_filter_FIX(
    opus_int16                      *LTP_res,
    const opus_int16                *x,
    const opus_int16                LTPCoef_Q14[ LTP_ORDER * MAX_NB_SUBFR ],
    const opus_int                  pitchL[ MAX_NB_SUBFR ],
    const opus_int32                invGains_Q16[ MAX_NB_SUBFR ],
    const opus_int                  subfr_length,
    const opus_int                  nb_subfr,
    const opus_int                  pre_length
)
{
    const opus_int16 *x_ptr, *x_lag_ptr;
    opus_int16   Btmp_Q14[ LTP_ORDER ];
    opus_int16   *LTP_res_ptr;
    opus_int     k, i;
    opus_int32   LTP_est;

    x_ptr = x;
    LTP_res_ptr = LTP_res;
    for( k = 0; k < nb_subfr; k++ ) {

        x_lag_ptr = x_ptr - pitchL[ k ];

        Btmp_Q14[ 0 ] = LTPCoef_Q14[ k * LTP_ORDER ];
        Btmp_Q14[ 1 ] = LTPCoef_Q14[ k * LTP_ORDER + 1 ];
        Btmp_Q14[ 2 ] = LTPCoef_Q14[ k * LTP_ORDER + 2 ];
        Btmp_Q14[ 3 ] = LTPCoef_Q14[ k * LTP_ORDER + 3 ];
        Btmp_Q14[ 4 ] = LTPCoef_Q14[ k * LTP_ORDER + 4 ];

        for( i = 0; i < subfr_length + pre_length; i++ ) {
            LTP_res_ptr[ i ] = x_ptr[ i ];

            LTP_est = silk_SMULBB( x_lag_ptr[ LTP_ORDER / 2 ], Btmp_Q14[ 0 ] );
            LTP_est = silk_SMLABB_ovflw( LTP_est, x_lag_ptr[ 1 ], Btmp_Q14[ 1 ] );
            LTP_est = silk_SMLABB_ovflw( LTP_est, x_lag_ptr[ 0 ], Btmp_Q14[ 2 ] );
            LTP_est = silk_SMLABB_ovflw( LTP_est, x_lag_ptr[ -1 ], Btmp_Q14[ 3 ] );
            LTP_est = silk_SMLABB_ovflw( LTP_est, x_lag_ptr[ -2 ], Btmp_Q14[ 4 ] );

            LTP_est = silk_RSHIFT_ROUND( LTP_est, 14 );

            LTP_res_ptr[ i ] = (opus_int16)silk_SAT16( (opus_int32)x_ptr[ i ] - LTP_est );

            LTP_res_ptr[ i ] = silk_SMULWB( invGains_Q16[ k ], LTP_res_ptr[ i ] );

            x_lag_ptr++;
        }

        LTP_res_ptr += subfr_length + pre_length;
        x_ptr       += subfr_length;
    }
}

