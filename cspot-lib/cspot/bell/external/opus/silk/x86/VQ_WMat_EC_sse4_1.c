

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include "main.h"
#include "celt/x86/x86cpu.h"

void silk_VQ_WMat_EC_sse4_1(
    opus_int8                   *ind,
    opus_int32                  *rate_dist_Q14,
    opus_int                    *gain_Q7,
    const opus_int16            *in_Q14,
    const opus_int32            *W_Q18,
    const opus_int8             *cb_Q7,
    const opus_uint8            *cb_gain_Q7,
    const opus_uint8            *cl_Q5,
    const opus_int              mu_Q9,
    const opus_int32            max_gain_Q7,
    opus_int                    L
)
{
    opus_int   k, gain_tmp_Q7;
    const opus_int8 *cb_row_Q7;
    opus_int16 diff_Q14[ 5 ];
    opus_int32 sum1_Q14, sum2_Q16;

    __m128i C_tmp1, C_tmp2, C_tmp3, C_tmp4, C_tmp5;

    *rate_dist_Q14 = silk_int32_MAX;
    cb_row_Q7 = cb_Q7;
    for( k = 0; k < L; k++ ) {
        gain_tmp_Q7 = cb_gain_Q7[k];

        diff_Q14[ 0 ] = in_Q14[ 0 ] - silk_LSHIFT( cb_row_Q7[ 0 ], 7 );

        C_tmp1 = OP_CVTEPI16_EPI32_M64( &in_Q14[ 1 ] );
        C_tmp2 = OP_CVTEPI8_EPI32_M32( &cb_row_Q7[ 1 ] );
        C_tmp2 = _mm_slli_epi32( C_tmp2, 7 );
        C_tmp1 = _mm_sub_epi32( C_tmp1, C_tmp2 );

        diff_Q14[ 1 ] = _mm_extract_epi16( C_tmp1, 0 );
        diff_Q14[ 2 ] = _mm_extract_epi16( C_tmp1, 2 );
        diff_Q14[ 3 ] = _mm_extract_epi16( C_tmp1, 4 );
        diff_Q14[ 4 ] = _mm_extract_epi16( C_tmp1, 6 );

        sum1_Q14 = silk_SMULBB( mu_Q9, cl_Q5[ k ] );

        sum1_Q14 = silk_ADD_LSHIFT32( sum1_Q14, silk_max( silk_SUB32( gain_tmp_Q7, max_gain_Q7 ), 0 ), 10 );

        silk_assert( sum1_Q14 >= 0 );

        C_tmp3 = _mm_loadu_si128( (__m128i *)(&W_Q18[ 1 ] ) );
        C_tmp4 = _mm_mul_epi32( C_tmp3, C_tmp1 );
        C_tmp4 = _mm_srli_si128( C_tmp4, 2 );

        C_tmp1 = _mm_shuffle_epi32( C_tmp1, _MM_SHUFFLE( 0, 3, 2, 1 ) );
        C_tmp3 = _mm_shuffle_epi32( C_tmp3, _MM_SHUFFLE( 0, 3, 2, 1 ) );

        C_tmp5 = _mm_mul_epi32( C_tmp3, C_tmp1 );
        C_tmp5 = _mm_srli_si128( C_tmp5, 2 );

        C_tmp5 = _mm_add_epi32( C_tmp4, C_tmp5 );
        C_tmp5 = _mm_slli_epi32( C_tmp5, 1 );

        C_tmp5 = _mm_add_epi32( C_tmp5, _mm_shuffle_epi32( C_tmp5, _MM_SHUFFLE( 0, 0, 0, 2 ) ) );
        sum2_Q16 = _mm_cvtsi128_si32( C_tmp5 );

        sum2_Q16 = silk_SMLAWB( sum2_Q16, W_Q18[  0 ], diff_Q14[ 0 ] );
        sum1_Q14 = silk_SMLAWB( sum1_Q14, sum2_Q16,    diff_Q14[ 0 ] );

        sum2_Q16 = silk_SMULWB(           W_Q18[  7 ], diff_Q14[ 2 ] );
        sum2_Q16 = silk_SMLAWB( sum2_Q16, W_Q18[  8 ], diff_Q14[ 3 ] );
        sum2_Q16 = silk_SMLAWB( sum2_Q16, W_Q18[  9 ], diff_Q14[ 4 ] );
        sum2_Q16 = silk_LSHIFT( sum2_Q16, 1 );
        sum2_Q16 = silk_SMLAWB( sum2_Q16, W_Q18[  6 ], diff_Q14[ 1 ] );
        sum1_Q14 = silk_SMLAWB( sum1_Q14, sum2_Q16,    diff_Q14[ 1 ] );

        sum2_Q16 = silk_SMULWB(           W_Q18[ 13 ], diff_Q14[ 3 ] );
        sum2_Q16 = silk_SMLAWB( sum2_Q16, W_Q18[ 14 ], diff_Q14[ 4 ] );
        sum2_Q16 = silk_LSHIFT( sum2_Q16, 1 );
        sum2_Q16 = silk_SMLAWB( sum2_Q16, W_Q18[ 12 ], diff_Q14[ 2 ] );
        sum1_Q14 = silk_SMLAWB( sum1_Q14, sum2_Q16,    diff_Q14[ 2 ] );

        sum2_Q16 = silk_SMULWB(           W_Q18[ 19 ], diff_Q14[ 4 ] );
        sum2_Q16 = silk_LSHIFT( sum2_Q16, 1 );
        sum2_Q16 = silk_SMLAWB( sum2_Q16, W_Q18[ 18 ], diff_Q14[ 3 ] );
        sum1_Q14 = silk_SMLAWB( sum1_Q14, sum2_Q16,    diff_Q14[ 3 ] );

        sum2_Q16 = silk_SMULWB(           W_Q18[ 24 ], diff_Q14[ 4 ] );
        sum1_Q14 = silk_SMLAWB( sum1_Q14, sum2_Q16,    diff_Q14[ 4 ] );

        silk_assert( sum1_Q14 >= 0 );

        if( sum1_Q14 < *rate_dist_Q14 ) {
            *rate_dist_Q14 = sum1_Q14;
            *ind = (opus_int8)k;
            *gain_Q7 = gain_tmp_Q7;
        }

        cb_row_Q7 += LTP_ORDER;
    }
}
