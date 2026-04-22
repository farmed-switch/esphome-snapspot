

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main.h"

void silk_NLSF_VQ(
    opus_int32                  err_Q24[],
    const opus_int16            in_Q15[],
    const opus_uint8            pCB_Q8[],
    const opus_int16            pWght_Q9[],
    const opus_int              K,
    const opus_int              LPC_order
)
{
    opus_int         i, m;
    opus_int32       diff_Q15, diffw_Q24, sum_error_Q24, pred_Q24;
    const opus_int16 *w_Q9_ptr;
    const opus_uint8 *cb_Q8_ptr;

    celt_assert( ( LPC_order & 1 ) == 0 );

    cb_Q8_ptr = pCB_Q8;
    w_Q9_ptr = pWght_Q9;
    for( i = 0; i < K; i++ ) {
        sum_error_Q24 = 0;
        pred_Q24 = 0;
        for( m = LPC_order-2; m >= 0; m -= 2 ) {

            diff_Q15 = silk_SUB_LSHIFT32( in_Q15[ m + 1 ], (opus_int32)cb_Q8_ptr[ m + 1 ], 7 );
            diffw_Q24 = silk_SMULBB( diff_Q15, w_Q9_ptr[ m + 1 ] );
            sum_error_Q24 = silk_ADD32( sum_error_Q24, silk_abs( silk_SUB_RSHIFT32( diffw_Q24, pred_Q24, 1 ) ) );
            pred_Q24 = diffw_Q24;

            diff_Q15 = silk_SUB_LSHIFT32( in_Q15[ m ], (opus_int32)cb_Q8_ptr[ m ], 7 );
            diffw_Q24 = silk_SMULBB( diff_Q15, w_Q9_ptr[ m ] );
            sum_error_Q24 = silk_ADD32( sum_error_Q24, silk_abs( silk_SUB_RSHIFT32( diffw_Q24, pred_Q24, 1 ) ) );
            pred_Q24 = diffw_Q24;

            silk_assert( sum_error_Q24 >= 0 );
        }
        err_Q24[ i ] = sum_error_Q24;
        cb_Q8_ptr += LPC_order;
        w_Q9_ptr += LPC_order;
    }
}
