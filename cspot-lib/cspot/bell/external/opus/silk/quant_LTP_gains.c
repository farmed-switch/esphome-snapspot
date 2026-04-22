

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main.h"
#include "tuning_parameters.h"

void silk_quant_LTP_gains(
    opus_int16                  B_Q14[ MAX_NB_SUBFR * LTP_ORDER ],
    opus_int8                   cbk_index[ MAX_NB_SUBFR ],
    opus_int8                   *periodicity_index,
    opus_int32                  *sum_log_gain_Q7,
    opus_int                    *pred_gain_dB_Q7,
    const opus_int32            XX_Q17[ MAX_NB_SUBFR*LTP_ORDER*LTP_ORDER ],
    const opus_int32            xX_Q17[ MAX_NB_SUBFR*LTP_ORDER ],
    const opus_int              subfr_len,
    const opus_int              nb_subfr,
    int                         arch
)
{
    opus_int             j, k, cbk_size;
    opus_int8            temp_idx[ MAX_NB_SUBFR ];
    const opus_uint8     *cl_ptr_Q5;
    const opus_int8      *cbk_ptr_Q7;
    const opus_uint8     *cbk_gain_ptr_Q7;
    const opus_int32     *XX_Q17_ptr, *xX_Q17_ptr;
    opus_int32           res_nrg_Q15_subfr, res_nrg_Q15, rate_dist_Q7_subfr, rate_dist_Q7, min_rate_dist_Q7;
    opus_int32           sum_log_gain_tmp_Q7, best_sum_log_gain_Q7, max_gain_Q7;
    opus_int             gain_Q7;

    min_rate_dist_Q7 = silk_int32_MAX;
    best_sum_log_gain_Q7 = 0;
    for( k = 0; k < 3; k++ ) {

        opus_int32 gain_safety = SILK_FIX_CONST( 0.4, 7 );

        cl_ptr_Q5  = silk_LTP_gain_BITS_Q5_ptrs[ k ];
        cbk_ptr_Q7 = silk_LTP_vq_ptrs_Q7[        k ];
        cbk_gain_ptr_Q7 = silk_LTP_vq_gain_ptrs_Q7[ k ];
        cbk_size   = silk_LTP_vq_sizes[          k ];

        XX_Q17_ptr = XX_Q17;
        xX_Q17_ptr = xX_Q17;

        res_nrg_Q15 = 0;
        rate_dist_Q7 = 0;
        sum_log_gain_tmp_Q7 = *sum_log_gain_Q7;
        for( j = 0; j < nb_subfr; j++ ) {
            max_gain_Q7 = silk_log2lin( ( SILK_FIX_CONST( MAX_SUM_LOG_GAIN_DB / 6.0, 7 ) - sum_log_gain_tmp_Q7 )
                                        + SILK_FIX_CONST( 7, 7 ) ) - gain_safety;
            silk_VQ_WMat_EC(
                &temp_idx[ j ],
                &res_nrg_Q15_subfr,
                &rate_dist_Q7_subfr,
                &gain_Q7,
                XX_Q17_ptr,
                xX_Q17_ptr,
                cbk_ptr_Q7,
                cbk_gain_ptr_Q7,
                cl_ptr_Q5,
                subfr_len,
                max_gain_Q7,
                cbk_size,
                arch
            );

            res_nrg_Q15  = silk_ADD_POS_SAT32( res_nrg_Q15, res_nrg_Q15_subfr );
            rate_dist_Q7 = silk_ADD_POS_SAT32( rate_dist_Q7, rate_dist_Q7_subfr );
            sum_log_gain_tmp_Q7 = silk_max(0, sum_log_gain_tmp_Q7
                                + silk_lin2log( gain_safety + gain_Q7 ) - SILK_FIX_CONST( 7, 7 ));

            XX_Q17_ptr += LTP_ORDER * LTP_ORDER;
            xX_Q17_ptr += LTP_ORDER;
        }

        if( rate_dist_Q7 <= min_rate_dist_Q7 ) {
            min_rate_dist_Q7 = rate_dist_Q7;
            *periodicity_index = (opus_int8)k;
            silk_memcpy( cbk_index, temp_idx, nb_subfr * sizeof( opus_int8 ) );
            best_sum_log_gain_Q7 = sum_log_gain_tmp_Q7;
        }
    }

    cbk_ptr_Q7 = silk_LTP_vq_ptrs_Q7[ *periodicity_index ];
    for( j = 0; j < nb_subfr; j++ ) {
        for( k = 0; k < LTP_ORDER; k++ ) {
            B_Q14[ j * LTP_ORDER + k ] = silk_LSHIFT( cbk_ptr_Q7[ cbk_index[ j ] * LTP_ORDER + k ], 7 );
        }
    }

    if( nb_subfr == 2 ) {
        res_nrg_Q15 = silk_RSHIFT32( res_nrg_Q15, 1 );
    } else {
        res_nrg_Q15 = silk_RSHIFT32( res_nrg_Q15, 2 );
    }

    *sum_log_gain_Q7 = best_sum_log_gain_Q7;
    *pred_gain_dB_Q7 = (opus_int)silk_SMULBB( -3, silk_lin2log( res_nrg_Q15 ) - ( 15 << 7 ) );
}
