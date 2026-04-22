

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main.h"

void silk_VQ_WMat_EC_c(
    opus_int8                   *ind,
    opus_int32                  *res_nrg_Q15,
    opus_int32                  *rate_dist_Q8,
    opus_int                    *gain_Q7,
    const opus_int32            *XX_Q17,
    const opus_int32            *xX_Q17,
    const opus_int8             *cb_Q7,
    const opus_uint8            *cb_gain_Q7,
    const opus_uint8            *cl_Q5,
    const opus_int              subfr_len,
    const opus_int32            max_gain_Q7,
    const opus_int              L
)
{
    opus_int   k, gain_tmp_Q7;
    const opus_int8 *cb_row_Q7;
    opus_int32 neg_xX_Q24[ 5 ];
    opus_int32 sum1_Q15, sum2_Q24;
    opus_int32 bits_res_Q8, bits_tot_Q8;

    neg_xX_Q24[ 0 ] = -silk_LSHIFT32( xX_Q17[ 0 ], 7 );
    neg_xX_Q24[ 1 ] = -silk_LSHIFT32( xX_Q17[ 1 ], 7 );
    neg_xX_Q24[ 2 ] = -silk_LSHIFT32( xX_Q17[ 2 ], 7 );
    neg_xX_Q24[ 3 ] = -silk_LSHIFT32( xX_Q17[ 3 ], 7 );
    neg_xX_Q24[ 4 ] = -silk_LSHIFT32( xX_Q17[ 4 ], 7 );

    *rate_dist_Q8 = silk_int32_MAX;
    *res_nrg_Q15 = silk_int32_MAX;
    cb_row_Q7 = cb_Q7;

    *ind = 0;
    for( k = 0; k < L; k++ ) {
        opus_int32 penalty;
        gain_tmp_Q7 = cb_gain_Q7[k];

        sum1_Q15 = SILK_FIX_CONST( 1.001, 15 );

        penalty = silk_LSHIFT32( silk_max( silk_SUB32( gain_tmp_Q7, max_gain_Q7 ), 0 ), 11 );

        sum2_Q24 = silk_MLA( neg_xX_Q24[ 0 ], XX_Q17[  1 ], cb_row_Q7[ 1 ] );
        sum2_Q24 = silk_MLA( sum2_Q24,        XX_Q17[  2 ], cb_row_Q7[ 2 ] );
        sum2_Q24 = silk_MLA( sum2_Q24,        XX_Q17[  3 ], cb_row_Q7[ 3 ] );
        sum2_Q24 = silk_MLA( sum2_Q24,        XX_Q17[  4 ], cb_row_Q7[ 4 ] );
        sum2_Q24 = silk_LSHIFT32( sum2_Q24, 1 );
        sum2_Q24 = silk_MLA( sum2_Q24,        XX_Q17[  0 ], cb_row_Q7[ 0 ] );
        sum1_Q15 = silk_SMLAWB( sum1_Q15,        sum2_Q24,  cb_row_Q7[ 0 ] );

        sum2_Q24 = silk_MLA( neg_xX_Q24[ 1 ], XX_Q17[  7 ], cb_row_Q7[ 2 ] );
        sum2_Q24 = silk_MLA( sum2_Q24,        XX_Q17[  8 ], cb_row_Q7[ 3 ] );
        sum2_Q24 = silk_MLA( sum2_Q24,        XX_Q17[  9 ], cb_row_Q7[ 4 ] );
        sum2_Q24 = silk_LSHIFT32( sum2_Q24, 1 );
        sum2_Q24 = silk_MLA( sum2_Q24,        XX_Q17[  6 ], cb_row_Q7[ 1 ] );
        sum1_Q15 = silk_SMLAWB( sum1_Q15,        sum2_Q24,  cb_row_Q7[ 1 ] );

        sum2_Q24 = silk_MLA( neg_xX_Q24[ 2 ], XX_Q17[ 13 ], cb_row_Q7[ 3 ] );
        sum2_Q24 = silk_MLA( sum2_Q24,        XX_Q17[ 14 ], cb_row_Q7[ 4 ] );
        sum2_Q24 = silk_LSHIFT32( sum2_Q24, 1 );
        sum2_Q24 = silk_MLA( sum2_Q24,        XX_Q17[ 12 ], cb_row_Q7[ 2 ] );
        sum1_Q15 = silk_SMLAWB( sum1_Q15,        sum2_Q24,  cb_row_Q7[ 2 ] );

        sum2_Q24 = silk_MLA( neg_xX_Q24[ 3 ], XX_Q17[ 19 ], cb_row_Q7[ 4 ] );
        sum2_Q24 = silk_LSHIFT32( sum2_Q24, 1 );
        sum2_Q24 = silk_MLA( sum2_Q24,        XX_Q17[ 18 ], cb_row_Q7[ 3 ] );
        sum1_Q15 = silk_SMLAWB( sum1_Q15,        sum2_Q24,  cb_row_Q7[ 3 ] );

        sum2_Q24 = silk_LSHIFT32( neg_xX_Q24[ 4 ], 1 );
        sum2_Q24 = silk_MLA( sum2_Q24,        XX_Q17[ 24 ], cb_row_Q7[ 4 ] );
        sum1_Q15 = silk_SMLAWB( sum1_Q15,        sum2_Q24,  cb_row_Q7[ 4 ] );

        if( sum1_Q15 >= 0 ) {

            bits_res_Q8 = silk_SMULBB( subfr_len, silk_lin2log( sum1_Q15 + penalty) - (15 << 7) );

            bits_tot_Q8 = silk_ADD_LSHIFT32( bits_res_Q8, cl_Q5[ k ], 3-1 );
            if( bits_tot_Q8 <= *rate_dist_Q8 ) {
                *rate_dist_Q8 = bits_tot_Q8;
                *res_nrg_Q15 = sum1_Q15 + penalty;
                *ind = (opus_int8)k;
                *gain_Q7 = gain_tmp_Q7;
            }
        }

        cb_row_Q7 += LTP_ORDER;
    }
}
