

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include "main.h"
#include "celt/x86/x86cpu.h"
#include "stack_alloc.h"

static OPUS_INLINE void silk_nsq_scale_states_sse4_1(
    const silk_encoder_state *psEncC,
    silk_nsq_state      *NSQ,
    const opus_int32    x_Q3[],
    opus_int32          x_sc_Q10[],
    const opus_int16    sLTP[],
    opus_int32          sLTP_Q15[],
    opus_int            subfr,
    const opus_int      LTP_scale_Q14,
    const opus_int32    Gains_Q16[ MAX_NB_SUBFR ],
    const opus_int      pitchL[ MAX_NB_SUBFR ],
    const opus_int      signal_type
);

static OPUS_INLINE void silk_noise_shape_quantizer_10_16_sse4_1(
    silk_nsq_state      *NSQ,
    opus_int            signalType,
    const opus_int32    x_sc_Q10[],
    opus_int8           pulses[],
    opus_int16          xq[],
    opus_int32          sLTP_Q15[],
    const opus_int16    a_Q12[],
    const opus_int16    b_Q14[],
    const opus_int16    AR_shp_Q13[],
    opus_int            lag,
    opus_int32          HarmShapeFIRPacked_Q14,
    opus_int            Tilt_Q14,
    opus_int32          LF_shp_Q14,
    opus_int32          Gain_Q16,
    opus_int            offset_Q10,
    opus_int            length,
    opus_int32          table[][4]
);

void silk_NSQ_sse4_1(
    const silk_encoder_state    *psEncC,
    silk_nsq_state              *NSQ,
    SideInfoIndices             *psIndices,
    const opus_int32            x_Q3[],
    opus_int8                   pulses[],
    const opus_int16            PredCoef_Q12[ 2 * MAX_LPC_ORDER ],
    const opus_int16            LTPCoef_Q14[ LTP_ORDER * MAX_NB_SUBFR ],
    const opus_int16            AR2_Q13[ MAX_NB_SUBFR * MAX_SHAPE_LPC_ORDER ],
    const opus_int              HarmShapeGain_Q14[ MAX_NB_SUBFR ],
    const opus_int              Tilt_Q14[ MAX_NB_SUBFR ],
    const opus_int32            LF_shp_Q14[ MAX_NB_SUBFR ],
    const opus_int32            Gains_Q16[ MAX_NB_SUBFR ],
    const opus_int              pitchL[ MAX_NB_SUBFR ],
    const opus_int              Lambda_Q10,
    const opus_int              LTP_scale_Q14
)
{
    opus_int            k, lag, start_idx, LSF_interpolation_flag;
    const opus_int16    *A_Q12, *B_Q14, *AR_shp_Q13;
    opus_int16          *pxq;
    VARDECL( opus_int32, sLTP_Q15 );
    VARDECL( opus_int16, sLTP );
    opus_int32          HarmShapeFIRPacked_Q14;
    opus_int            offset_Q10;
    VARDECL( opus_int32, x_sc_Q10 );

    opus_int32   table[ 64 ][ 4 ];
    opus_int32   tmp1;
    opus_int32   q1_Q10, q2_Q10, rd1_Q20, rd2_Q20;

    SAVE_STACK;

    NSQ->rand_seed = psIndices->Seed;

    lag = NSQ->lagPrev;

    silk_assert( NSQ->prev_gain_Q16 != 0 );

    offset_Q10 = silk_Quantization_Offsets_Q10[ psIndices->signalType >> 1 ][ psIndices->quantOffsetType ];

    q1_Q10  = offset_Q10;
    q2_Q10  = offset_Q10 + ( 1024 - QUANT_LEVEL_ADJUST_Q10 );
    rd1_Q20 = q1_Q10 * Lambda_Q10;
    rd2_Q20 = q2_Q10 * Lambda_Q10;

    table[ 32 ][ 0 ] = q1_Q10;
    table[ 32 ][ 1 ] = q2_Q10;
    table[ 32 ][ 2 ] = 2 * (q1_Q10 - q2_Q10);
    table[ 32 ][ 3 ] = (rd1_Q20 - rd2_Q20) + (q1_Q10 * q1_Q10 - q2_Q10 * q2_Q10);

    q1_Q10  = offset_Q10 - ( 1024 - QUANT_LEVEL_ADJUST_Q10 );
    q2_Q10  = offset_Q10;
    rd1_Q20 = - q1_Q10 * Lambda_Q10;
    rd2_Q20 = q2_Q10 * Lambda_Q10;

    table[ 31 ][ 0 ] = q1_Q10;
    table[ 31 ][ 1 ] = q2_Q10;
    table[ 31 ][ 2 ] = 2 * (q1_Q10 - q2_Q10);
    table[ 31 ][ 3 ] = (rd1_Q20 - rd2_Q20) + (q1_Q10 * q1_Q10 - q2_Q10 * q2_Q10);

    for (k = 1; k <= 31; k++)
    {
        tmp1 = offset_Q10 + silk_LSHIFT( k, 10 );

        q1_Q10  = tmp1 - QUANT_LEVEL_ADJUST_Q10;
        q2_Q10  = tmp1 - QUANT_LEVEL_ADJUST_Q10 + 1024;
        rd1_Q20 = q1_Q10 * Lambda_Q10;
        rd2_Q20 = q2_Q10 * Lambda_Q10;

        table[ 32 + k ][ 0 ] = q1_Q10;
        table[ 32 + k ][ 1 ] = q2_Q10;
        table[ 32 + k ][ 2 ] = 2 * (q1_Q10 - q2_Q10);
        table[ 32 + k ][ 3 ] = (rd1_Q20 - rd2_Q20) + (q1_Q10 * q1_Q10 - q2_Q10 * q2_Q10);
    }

    for (k = -32; k <= -2; k++)
    {
        tmp1 = offset_Q10 + silk_LSHIFT( k, 10 );

        q1_Q10  = tmp1 + QUANT_LEVEL_ADJUST_Q10;
        q2_Q10  = tmp1 + QUANT_LEVEL_ADJUST_Q10 + 1024;
        rd1_Q20 = - q1_Q10 * Lambda_Q10;
        rd2_Q20 = - q2_Q10 * Lambda_Q10;

        table[ 32 + k ][ 0 ] = q1_Q10;
        table[ 32 + k ][ 1 ] = q2_Q10;
        table[ 32 + k ][ 2 ] = 2 * (q1_Q10 - q2_Q10);
        table[ 32 + k ][ 3 ] = (rd1_Q20 - rd2_Q20) + (q1_Q10 * q1_Q10 - q2_Q10 * q2_Q10);
    }

    if( psIndices->NLSFInterpCoef_Q2 == 4 ) {
        LSF_interpolation_flag = 0;
    } else {
        LSF_interpolation_flag = 1;
    }

    ALLOC( sLTP_Q15,
           psEncC->ltp_mem_length + psEncC->frame_length, opus_int32 );
    ALLOC( sLTP, psEncC->ltp_mem_length + psEncC->frame_length, opus_int16 );
    ALLOC( x_sc_Q10, psEncC->subfr_length, opus_int32 );

    NSQ->sLTP_shp_buf_idx = psEncC->ltp_mem_length;
    NSQ->sLTP_buf_idx     = psEncC->ltp_mem_length;
    pxq                   = &NSQ->xq[ psEncC->ltp_mem_length ];
    for( k = 0; k < psEncC->nb_subfr; k++ ) {
        A_Q12      = &PredCoef_Q12[ (( k >> 1 ) | ( 1 - LSF_interpolation_flag )) * MAX_LPC_ORDER ];
        B_Q14      = &LTPCoef_Q14[ k * LTP_ORDER ];
        AR_shp_Q13 = &AR2_Q13[     k * MAX_SHAPE_LPC_ORDER ];

        silk_assert( HarmShapeGain_Q14[ k ] >= 0 );
        HarmShapeFIRPacked_Q14  =                          silk_RSHIFT( HarmShapeGain_Q14[ k ], 2 );
        HarmShapeFIRPacked_Q14 |= silk_LSHIFT( (opus_int32)silk_RSHIFT( HarmShapeGain_Q14[ k ], 1 ), 16 );

        NSQ->rewhite_flag = 0;
        if( psIndices->signalType == TYPE_VOICED ) {

            lag = pitchL[ k ];

            if( ( k & ( 3 - silk_LSHIFT( LSF_interpolation_flag, 1 ) ) ) == 0 ) {

                start_idx = psEncC->ltp_mem_length - lag - psEncC->predictLPCOrder - LTP_ORDER / 2;
                celt_assert( start_idx > 0 );

                silk_LPC_analysis_filter( &sLTP[ start_idx ], &NSQ->xq[ start_idx + k * psEncC->subfr_length ],
                    A_Q12, psEncC->ltp_mem_length - start_idx, psEncC->predictLPCOrder, psEncC->arch );

                NSQ->rewhite_flag = 1;
                NSQ->sLTP_buf_idx = psEncC->ltp_mem_length;
            }
        }

        silk_nsq_scale_states_sse4_1( psEncC, NSQ, x_Q3, x_sc_Q10, sLTP, sLTP_Q15, k, LTP_scale_Q14, Gains_Q16, pitchL, psIndices->signalType );

        if ( opus_likely( ( 10 == psEncC->shapingLPCOrder ) && ( 16 == psEncC->predictLPCOrder) ) )
        {
            silk_noise_shape_quantizer_10_16_sse4_1( NSQ, psIndices->signalType, x_sc_Q10, pulses, pxq, sLTP_Q15, A_Q12, B_Q14,
                AR_shp_Q13, lag, HarmShapeFIRPacked_Q14, Tilt_Q14[ k ], LF_shp_Q14[ k ], Gains_Q16[ k ],
                offset_Q10, psEncC->subfr_length, &(table[32]) );
        }
        else
        {
            silk_noise_shape_quantizer( NSQ, psIndices->signalType, x_sc_Q10, pulses, pxq, sLTP_Q15, A_Q12, B_Q14,
                AR_shp_Q13, lag, HarmShapeFIRPacked_Q14, Tilt_Q14[ k ], LF_shp_Q14[ k ], Gains_Q16[ k ], Lambda_Q10,
                offset_Q10, psEncC->subfr_length, psEncC->shapingLPCOrder, psEncC->predictLPCOrder, psEncC->arch );
        }

        x_Q3   += psEncC->subfr_length;
        pulses += psEncC->subfr_length;
        pxq    += psEncC->subfr_length;
    }

    NSQ->lagPrev = pitchL[ psEncC->nb_subfr - 1 ];

    silk_memmove( NSQ->xq,           &NSQ->xq[           psEncC->frame_length ], psEncC->ltp_mem_length * sizeof( opus_int16 ) );
    silk_memmove( NSQ->sLTP_shp_Q14, &NSQ->sLTP_shp_Q14[ psEncC->frame_length ], psEncC->ltp_mem_length * sizeof( opus_int32 ) );
    RESTORE_STACK;
}

static OPUS_INLINE void silk_noise_shape_quantizer_10_16_sse4_1(
    silk_nsq_state      *NSQ,
    opus_int            signalType,
    const opus_int32    x_sc_Q10[],
    opus_int8           pulses[],
    opus_int16          xq[],
    opus_int32          sLTP_Q15[],
    const opus_int16    a_Q12[],
    const opus_int16    b_Q14[],
    const opus_int16    AR_shp_Q13[],
    opus_int            lag,
    opus_int32          HarmShapeFIRPacked_Q14,
    opus_int            Tilt_Q14,
    opus_int32          LF_shp_Q14,
    opus_int32          Gain_Q16,
    opus_int            offset_Q10,
    opus_int            length,
    opus_int32          table[][4]
)
{
    opus_int     i;
    opus_int32   LTP_pred_Q13, LPC_pred_Q10, n_AR_Q12, n_LTP_Q13;
    opus_int32   n_LF_Q12, r_Q10, q1_Q0, q1_Q10, q2_Q10;
    opus_int32   exc_Q14, LPC_exc_Q14, xq_Q14, Gain_Q10;
    opus_int32   tmp1, tmp2, sLF_AR_shp_Q14;
    opus_int32   *psLPC_Q14, *shp_lag_ptr, *pred_lag_ptr;

    __m128i xmm_tempa, xmm_tempb;

    __m128i xmm_one;

    __m128i psLPC_Q14_hi_01234567, psLPC_Q14_hi_89ABCDEF;
    __m128i psLPC_Q14_lo_01234567, psLPC_Q14_lo_89ABCDEF;
    __m128i a_Q12_01234567,        a_Q12_89ABCDEF;

    __m128i sAR2_Q14_hi_76543210, sAR2_Q14_lo_76543210;
    __m128i AR_shp_Q13_76543210;

    shp_lag_ptr  = &NSQ->sLTP_shp_Q14[ NSQ->sLTP_shp_buf_idx - lag + HARM_SHAPE_FIR_TAPS / 2 ];
    pred_lag_ptr = &sLTP_Q15[ NSQ->sLTP_buf_idx - lag + LTP_ORDER / 2 ];
    Gain_Q10     = silk_RSHIFT( Gain_Q16, 6 );

    psLPC_Q14 = &NSQ->sLPC_Q14[ NSQ_LPC_BUF_LENGTH - 1 ];

    sLF_AR_shp_Q14 = NSQ->sLF_AR_shp_Q14;
    xq_Q14         = psLPC_Q14[ 0 ];
    LTP_pred_Q13   = 0;

    xmm_one = _mm_set_epi8( 1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14 );

    a_Q12_01234567 = _mm_loadu_si128( (__m128i *)(&a_Q12[ 0 ] ) );

    a_Q12_89ABCDEF = _mm_loadu_si128( (__m128i *)(&a_Q12[ 8 ] ) );

    a_Q12_01234567 = _mm_shuffle_epi8( a_Q12_01234567, xmm_one );
    a_Q12_89ABCDEF = _mm_shuffle_epi8( a_Q12_89ABCDEF, xmm_one );

    AR_shp_Q13_76543210 = _mm_loadu_si128( (__m128i *)(&AR_shp_Q13[0] ) );

    xmm_one = _mm_set_epi8(15, 14, 11, 10, 7, 6, 3, 2, 13, 12, 9, 8, 5, 4, 1, 0 );

    xmm_tempa = _mm_loadu_si128( (__m128i *)(&psLPC_Q14[-16]) );
    xmm_tempb = _mm_loadu_si128( (__m128i *)(&psLPC_Q14[-12]) );

    xmm_tempa = _mm_shuffle_epi8( xmm_tempa, xmm_one );
    xmm_tempb = _mm_shuffle_epi8( xmm_tempb, xmm_one );

    psLPC_Q14_hi_89ABCDEF = _mm_unpackhi_epi64( xmm_tempa, xmm_tempb );
    psLPC_Q14_lo_89ABCDEF = _mm_unpacklo_epi64( xmm_tempa, xmm_tempb );

    xmm_tempa = _mm_loadu_si128( (__m128i *)(&psLPC_Q14[ -8 ]) );
    xmm_tempb = _mm_loadu_si128( (__m128i *)(&psLPC_Q14[ -4 ]) );

    xmm_tempa = _mm_shuffle_epi8( xmm_tempa, xmm_one );
    xmm_tempb = _mm_shuffle_epi8( xmm_tempb, xmm_one );

    psLPC_Q14_hi_01234567 = _mm_unpackhi_epi64( xmm_tempa, xmm_tempb );
    psLPC_Q14_lo_01234567 = _mm_unpacklo_epi64( xmm_tempa, xmm_tempb );

    xmm_tempa = _mm_loadu_si128( (__m128i *)(&(NSQ->sAR2_Q14[ 0 ]) ) );
    xmm_tempb = _mm_loadu_si128( (__m128i *)(&(NSQ->sAR2_Q14[ 4 ]) ) );

    xmm_tempa = _mm_shuffle_epi8( xmm_tempa, xmm_one );
    xmm_tempb = _mm_shuffle_epi8( xmm_tempb, xmm_one );

    sAR2_Q14_hi_76543210 = _mm_unpackhi_epi64( xmm_tempa, xmm_tempb );
    sAR2_Q14_lo_76543210 = _mm_unpacklo_epi64( xmm_tempa, xmm_tempb );

    xmm_one = _mm_set1_epi16(1);

    for( i = 0; i < length; i++ )
    {

        __m128i xmm_hi_07, xmm_hi_8F, xmm_lo_07, xmm_lo_8F;

        LPC_pred_Q10 = 8;

        psLPC_Q14_hi_89ABCDEF = _mm_alignr_epi8( psLPC_Q14_hi_01234567, psLPC_Q14_hi_89ABCDEF, 2 );
        psLPC_Q14_lo_89ABCDEF = _mm_alignr_epi8( psLPC_Q14_lo_01234567, psLPC_Q14_lo_89ABCDEF, 2 );

        psLPC_Q14_hi_01234567 = _mm_srli_si128( psLPC_Q14_hi_01234567, 2 );
        psLPC_Q14_lo_01234567 = _mm_srli_si128( psLPC_Q14_lo_01234567, 2 );

        psLPC_Q14_hi_01234567 = _mm_insert_epi16( psLPC_Q14_hi_01234567, (xq_Q14 >> 16), 7 );
        psLPC_Q14_lo_01234567 = _mm_insert_epi16( psLPC_Q14_lo_01234567, (xq_Q14),       7 );

        xmm_hi_07 = _mm_madd_epi16( psLPC_Q14_hi_01234567, a_Q12_01234567 );
        xmm_hi_8F = _mm_madd_epi16( psLPC_Q14_hi_89ABCDEF, a_Q12_89ABCDEF );

        xmm_tempa = _mm_cmpgt_epi16( _mm_setzero_si128(), psLPC_Q14_lo_01234567 );
        xmm_tempb = _mm_cmpgt_epi16( _mm_setzero_si128(), psLPC_Q14_lo_89ABCDEF );

        xmm_tempa = _mm_and_si128( xmm_tempa, a_Q12_01234567 );
        xmm_tempb = _mm_and_si128( xmm_tempb, a_Q12_89ABCDEF );

        xmm_lo_07 = _mm_mulhi_epi16( psLPC_Q14_lo_01234567, a_Q12_01234567 );
        xmm_lo_8F = _mm_mulhi_epi16( psLPC_Q14_lo_89ABCDEF, a_Q12_89ABCDEF );

        xmm_lo_07 = _mm_add_epi16( xmm_lo_07, xmm_tempa );
        xmm_lo_8F = _mm_add_epi16( xmm_lo_8F, xmm_tempb );

        xmm_lo_07 = _mm_madd_epi16( xmm_lo_07, xmm_one );
        xmm_lo_8F = _mm_madd_epi16( xmm_lo_8F, xmm_one );

        xmm_hi_07 = _mm_add_epi32( xmm_hi_07, xmm_hi_8F );
        xmm_lo_07 = _mm_add_epi32( xmm_lo_07, xmm_lo_8F );

        xmm_hi_07 = _mm_add_epi32( xmm_hi_07, xmm_lo_07 );

        xmm_hi_07 = _mm_add_epi32( xmm_hi_07, _mm_unpackhi_epi64(xmm_hi_07, xmm_hi_07 ) );
        xmm_hi_07 = _mm_add_epi32( xmm_hi_07, _mm_shufflelo_epi16(xmm_hi_07, 0x0E ) );

        LPC_pred_Q10 += _mm_cvtsi128_si32( xmm_hi_07 );

        if ( opus_likely( signalType == TYPE_VOICED ) ) {

            LTP_pred_Q13 = 2;
            {
                __m128i b_Q14_3210, b_Q14_0123, pred_lag_ptr_0123;

                b_Q14_3210 = OP_CVTEPI16_EPI32_M64( b_Q14 );
                b_Q14_0123 = _mm_shuffle_epi32( b_Q14_3210, 0x1B );

                pred_lag_ptr_0123 = _mm_loadu_si128( (__m128i *)(&pred_lag_ptr[ -3 ] ) );

                xmm_tempa = _mm_shuffle_epi32( pred_lag_ptr_0123, 0x1B );

                xmm_tempa = _mm_mul_epi32( xmm_tempa, b_Q14_3210 );

                xmm_tempa = _mm_srli_si128( xmm_tempa, 2 );

                pred_lag_ptr_0123 = _mm_mul_epi32( pred_lag_ptr_0123, b_Q14_0123 );
                pred_lag_ptr_0123 = _mm_srli_si128( pred_lag_ptr_0123, 2 );

                pred_lag_ptr_0123 = _mm_add_epi32( pred_lag_ptr_0123, xmm_tempa );

                xmm_tempa = _mm_shuffle_epi32( pred_lag_ptr_0123, _MM_SHUFFLE( 0, 0, 3, 2 ) );
                xmm_tempa = _mm_add_epi32( xmm_tempa, pred_lag_ptr_0123 );

                LTP_pred_Q13 += _mm_cvtsi128_si32( xmm_tempa );

                LTP_pred_Q13 = silk_SMLAWB( LTP_pred_Q13, pred_lag_ptr[ -4 ], b_Q14[ 4 ] );
                pred_lag_ptr++;
            }
        }

        NSQ->sAR2_Q14[ 9 ] = NSQ->sAR2_Q14[ 8 ];
        NSQ->sAR2_Q14[ 8 ] = _mm_cvtsi128_si32( _mm_srli_si128(_mm_unpackhi_epi16( sAR2_Q14_lo_76543210, sAR2_Q14_hi_76543210 ), 12 ) );

        sAR2_Q14_hi_76543210 = _mm_slli_si128( sAR2_Q14_hi_76543210, 2 );
        sAR2_Q14_lo_76543210 = _mm_slli_si128( sAR2_Q14_lo_76543210, 2 );

        sAR2_Q14_hi_76543210 = _mm_insert_epi16( sAR2_Q14_hi_76543210, (xq_Q14 >> 16), 0 );
        sAR2_Q14_lo_76543210 = _mm_insert_epi16( sAR2_Q14_lo_76543210, (xq_Q14),       0 );

        xmm_hi_07 = _mm_madd_epi16( sAR2_Q14_hi_76543210, AR_shp_Q13_76543210 );

        xmm_tempa = _mm_cmpgt_epi16( _mm_setzero_si128(), sAR2_Q14_lo_76543210 );
        xmm_tempa = _mm_and_si128( xmm_tempa, AR_shp_Q13_76543210 );

        xmm_lo_07 = _mm_mulhi_epi16( sAR2_Q14_lo_76543210, AR_shp_Q13_76543210 );
        xmm_lo_07 = _mm_add_epi16( xmm_lo_07, xmm_tempa );

        xmm_lo_07 = _mm_madd_epi16( xmm_lo_07, xmm_one );

        xmm_hi_07 = _mm_add_epi32( xmm_hi_07, xmm_lo_07 );

        xmm_hi_07 = _mm_add_epi32( xmm_hi_07, _mm_unpackhi_epi64(xmm_hi_07, xmm_hi_07 ) );
        xmm_hi_07 = _mm_add_epi32( xmm_hi_07, _mm_shufflelo_epi16(xmm_hi_07, 0x0E ) );

        n_AR_Q12 = 5 + _mm_cvtsi128_si32( xmm_hi_07 );

        n_AR_Q12 = silk_SMLAWB( n_AR_Q12, NSQ->sAR2_Q14[ 8 ], AR_shp_Q13[ 8 ] );
        n_AR_Q12 = silk_SMLAWB( n_AR_Q12, NSQ->sAR2_Q14[ 9 ], AR_shp_Q13[ 9 ] );

        n_AR_Q12 = silk_LSHIFT32( n_AR_Q12, 1 );
        n_AR_Q12 = silk_SMLAWB( n_AR_Q12, sLF_AR_shp_Q14, Tilt_Q14 );

        n_LF_Q12 = silk_SMULWB( NSQ->sLTP_shp_Q14[ NSQ->sLTP_shp_buf_idx - 1 ], LF_shp_Q14 );
        n_LF_Q12 = silk_SMLAWT( n_LF_Q12, sLF_AR_shp_Q14, LF_shp_Q14 );

        silk_assert( lag > 0 || signalType != TYPE_VOICED );

        tmp1 = silk_SUB32( silk_LSHIFT32( LPC_pred_Q10, 2 ), n_AR_Q12 );
        tmp1 = silk_SUB32( tmp1, n_LF_Q12 );
        if( lag > 0 ) {

            n_LTP_Q13 = silk_SMULWB( silk_ADD32( shp_lag_ptr[ 0 ], shp_lag_ptr[ -2 ] ), HarmShapeFIRPacked_Q14 );
            n_LTP_Q13 = silk_SMLAWT( n_LTP_Q13, shp_lag_ptr[ -1 ],                      HarmShapeFIRPacked_Q14 );
            n_LTP_Q13 = silk_LSHIFT( n_LTP_Q13, 1 );
            shp_lag_ptr++;

            tmp2 = silk_SUB32( LTP_pred_Q13, n_LTP_Q13 );
            tmp1 = silk_ADD_LSHIFT32( tmp2, tmp1, 1 );
            tmp1 = silk_RSHIFT_ROUND( tmp1, 3 );
        } else {
            tmp1 = silk_RSHIFT_ROUND( tmp1, 2 );
        }

        r_Q10 = silk_SUB32( x_sc_Q10[ i ], tmp1 );

        NSQ->rand_seed = silk_RAND( NSQ->rand_seed );

        tmp2 = -r_Q10;
        if ( NSQ->rand_seed < 0 ) r_Q10 = tmp2;

        r_Q10 = silk_LIMIT_32( r_Q10, -(31 << 10), 30 << 10 );

        q1_Q10 = silk_SUB32( r_Q10, offset_Q10 );
        q1_Q0 = silk_RSHIFT( q1_Q10, 10 );

        q1_Q10 = table[q1_Q0][0];
        q2_Q10 = table[q1_Q0][1];

        if (r_Q10 * table[q1_Q0][2] - table[q1_Q0][3] < 0)
        {
            q1_Q10 = q2_Q10;
        }

        pulses[ i ] = (opus_int8)silk_RSHIFT_ROUND( q1_Q10, 10 );

        exc_Q14 = silk_LSHIFT( q1_Q10, 4 );

        tmp2 = -exc_Q14;
        if ( NSQ->rand_seed < 0 ) exc_Q14 = tmp2;

        LPC_exc_Q14 = silk_ADD_LSHIFT32( exc_Q14, LTP_pred_Q13, 1 );
        xq_Q14      = silk_ADD_LSHIFT32( LPC_exc_Q14, LPC_pred_Q10, 4 );

        psLPC_Q14++;
        *psLPC_Q14 = xq_Q14;
        sLF_AR_shp_Q14 = silk_SUB_LSHIFT32( xq_Q14, n_AR_Q12, 2 );

        NSQ->sLTP_shp_Q14[ NSQ->sLTP_shp_buf_idx ] = silk_SUB_LSHIFT32( sLF_AR_shp_Q14, n_LF_Q12, 2 );
        sLTP_Q15[ NSQ->sLTP_buf_idx ] = silk_LSHIFT( LPC_exc_Q14, 1 );
        NSQ->sLTP_shp_buf_idx++;
        NSQ->sLTP_buf_idx++;

        NSQ->rand_seed = silk_ADD32_ovflw( NSQ->rand_seed, pulses[ i ] );
    }

    NSQ->sLF_AR_shp_Q14 = sLF_AR_shp_Q14;

    psLPC_Q14 = &NSQ->sLPC_Q14[ NSQ_LPC_BUF_LENGTH ];

    xmm_tempa = _mm_unpackhi_epi16( sAR2_Q14_lo_76543210, sAR2_Q14_hi_76543210 );
    xmm_tempb = _mm_unpacklo_epi16( sAR2_Q14_lo_76543210, sAR2_Q14_hi_76543210 );
    _mm_storeu_si128( (__m128i *)(&NSQ->sAR2_Q14[ 4 ]), xmm_tempa );
    _mm_storeu_si128( (__m128i *)(&NSQ->sAR2_Q14[ 0 ]), xmm_tempb );

    {
        __m128i xmm_Gain_Q10;
        __m128i xmm_xq_Q14_3210, xmm_xq_Q14_x3x1, xmm_xq_Q14_7654, xmm_xq_Q14_x7x5;

        xmm_tempa = _mm_set1_epi32( (1 << 7) );

        xmm_Gain_Q10 = _mm_set1_epi32( Gain_Q10 );

        for (i = 0; i < length - 7; i += 8)
        {
            xmm_xq_Q14_3210 = _mm_loadu_si128( (__m128i *)(&(psLPC_Q14[ i + 0 ] ) ) );
            xmm_xq_Q14_7654 = _mm_loadu_si128( (__m128i *)(&(psLPC_Q14[ i + 4 ] ) ) );

            xmm_xq_Q14_x3x1 = _mm_shuffle_epi32( xmm_xq_Q14_3210, _MM_SHUFFLE( 0, 3, 2, 1 ) );

            xmm_xq_Q14_x7x5 = _mm_shuffle_epi32( xmm_xq_Q14_7654, _MM_SHUFFLE( 0, 3, 2, 1 ) );

            xmm_xq_Q14_3210 = _mm_mul_epi32( xmm_xq_Q14_3210, xmm_Gain_Q10 );
            xmm_xq_Q14_x3x1 = _mm_mul_epi32( xmm_xq_Q14_x3x1, xmm_Gain_Q10 );
            xmm_xq_Q14_7654 = _mm_mul_epi32( xmm_xq_Q14_7654, xmm_Gain_Q10 );
            xmm_xq_Q14_x7x5 = _mm_mul_epi32( xmm_xq_Q14_x7x5, xmm_Gain_Q10 );

            xmm_xq_Q14_3210 = _mm_srli_epi64( xmm_xq_Q14_3210, 16 );
            xmm_xq_Q14_x3x1 = _mm_slli_epi64( xmm_xq_Q14_x3x1, 16 );
            xmm_xq_Q14_7654 = _mm_srli_epi64( xmm_xq_Q14_7654, 16 );
            xmm_xq_Q14_x7x5 = _mm_slli_epi64( xmm_xq_Q14_x7x5, 16 );

            xmm_xq_Q14_3210 = _mm_blend_epi16( xmm_xq_Q14_3210, xmm_xq_Q14_x3x1, 0xCC );
            xmm_xq_Q14_7654 = _mm_blend_epi16( xmm_xq_Q14_7654, xmm_xq_Q14_x7x5, 0xCC );

            xmm_xq_Q14_3210 = _mm_add_epi32( xmm_xq_Q14_3210, xmm_tempa );
            xmm_xq_Q14_7654 = _mm_add_epi32( xmm_xq_Q14_7654, xmm_tempa );

            xmm_xq_Q14_3210 = _mm_srai_epi32( xmm_xq_Q14_3210, 8 );
            xmm_xq_Q14_7654 = _mm_srai_epi32( xmm_xq_Q14_7654, 8 );

            xmm_xq_Q14_3210 = _mm_packs_epi32( xmm_xq_Q14_3210, xmm_xq_Q14_7654 );

            _mm_storeu_si128( (__m128i *)(&xq[ i ] ), xmm_xq_Q14_3210 );
        }
    }
    for ( ; i < length; i++)
    {
        xq[i] = (opus_int16)silk_SAT16( silk_RSHIFT_ROUND( silk_SMULWW( psLPC_Q14[ i ], Gain_Q10 ), 8 ) );
    }

    silk_memcpy( NSQ->sLPC_Q14, &NSQ->sLPC_Q14[ length ], NSQ_LPC_BUF_LENGTH * sizeof( opus_int32 ) );
}

static OPUS_INLINE void silk_nsq_scale_states_sse4_1(
    const silk_encoder_state *psEncC,
    silk_nsq_state      *NSQ,
    const opus_int32    x_Q3[],
    opus_int32          x_sc_Q10[],
    const opus_int16    sLTP[],
    opus_int32          sLTP_Q15[],
    opus_int            subfr,
    const opus_int      LTP_scale_Q14,
    const opus_int32    Gains_Q16[ MAX_NB_SUBFR ],
    const opus_int      pitchL[ MAX_NB_SUBFR ],
    const opus_int      signal_type
)
{
    opus_int   i, lag;
    opus_int32 gain_adj_Q16, inv_gain_Q31, inv_gain_Q23;
    __m128i xmm_inv_gain_Q23, xmm_x_Q3_x2x0, xmm_x_Q3_x3x1;

    lag          = pitchL[ subfr ];
    inv_gain_Q31 = silk_INVERSE32_varQ( silk_max( Gains_Q16[ subfr ], 1 ), 47 );
    silk_assert( inv_gain_Q31 != 0 );

    if( Gains_Q16[ subfr ] != NSQ->prev_gain_Q16 ) {
        gain_adj_Q16 =  silk_DIV32_varQ( NSQ->prev_gain_Q16, Gains_Q16[ subfr ], 16 );
    } else {
        gain_adj_Q16 = (opus_int32)1 << 16;
    }

    inv_gain_Q23 = silk_RSHIFT_ROUND( inv_gain_Q31, 8 );

    xmm_inv_gain_Q23 = _mm_set1_epi32(inv_gain_Q23);

    for( i = 0; i < psEncC->subfr_length - 3; i += 4 ) {
        xmm_x_Q3_x2x0 = _mm_loadu_si128( (__m128i *)(&(x_Q3[ i ] ) ) );

        xmm_x_Q3_x3x1 = _mm_shuffle_epi32( xmm_x_Q3_x2x0, _MM_SHUFFLE( 0, 3, 2, 1 ) );

        xmm_x_Q3_x2x0 = _mm_mul_epi32( xmm_x_Q3_x2x0, xmm_inv_gain_Q23 );
        xmm_x_Q3_x3x1 = _mm_mul_epi32( xmm_x_Q3_x3x1, xmm_inv_gain_Q23 );

        xmm_x_Q3_x2x0 = _mm_srli_epi64( xmm_x_Q3_x2x0, 16 );
        xmm_x_Q3_x3x1 = _mm_slli_epi64( xmm_x_Q3_x3x1, 16 );

        xmm_x_Q3_x2x0 = _mm_blend_epi16( xmm_x_Q3_x2x0, xmm_x_Q3_x3x1, 0xCC );

        _mm_storeu_si128( (__m128i *)(&(x_sc_Q10[ i ] ) ), xmm_x_Q3_x2x0 );
    }

    for( ; i < psEncC->subfr_length; i++ ) {
        x_sc_Q10[ i ] = silk_SMULWW( x_Q3[ i ], inv_gain_Q23 );
    }

    NSQ->prev_gain_Q16 = Gains_Q16[ subfr ];

    if( NSQ->rewhite_flag ) {
        if( subfr == 0 ) {

            inv_gain_Q31 = silk_LSHIFT( silk_SMULWB( inv_gain_Q31, LTP_scale_Q14 ), 2 );
        }
        for( i = NSQ->sLTP_buf_idx - lag - LTP_ORDER / 2; i < NSQ->sLTP_buf_idx; i++ ) {
            silk_assert( i < MAX_FRAME_LENGTH );
            sLTP_Q15[ i ] = silk_SMULWB( inv_gain_Q31, sLTP[ i ] );
        }
    }

    if( gain_adj_Q16 != (opus_int32)1 << 16 ) {

        __m128i xmm_gain_adj_Q16, xmm_sLTP_shp_Q14_x2x0, xmm_sLTP_shp_Q14_x3x1;

        xmm_gain_adj_Q16 = _mm_set1_epi32(gain_adj_Q16);

        for( i = NSQ->sLTP_shp_buf_idx - psEncC->ltp_mem_length; i < NSQ->sLTP_shp_buf_idx - 3; i += 4 )
        {
            xmm_sLTP_shp_Q14_x2x0 = _mm_loadu_si128( (__m128i *)(&(NSQ->sLTP_shp_Q14[ i ] ) ) );

            xmm_sLTP_shp_Q14_x3x1 = _mm_shuffle_epi32( xmm_sLTP_shp_Q14_x2x0, _MM_SHUFFLE( 0, 3, 2, 1 ) );

            xmm_sLTP_shp_Q14_x2x0 = _mm_mul_epi32( xmm_sLTP_shp_Q14_x2x0, xmm_gain_adj_Q16 );
            xmm_sLTP_shp_Q14_x3x1 = _mm_mul_epi32( xmm_sLTP_shp_Q14_x3x1, xmm_gain_adj_Q16 );

            xmm_sLTP_shp_Q14_x2x0 = _mm_srli_epi64( xmm_sLTP_shp_Q14_x2x0, 16 );
            xmm_sLTP_shp_Q14_x3x1 = _mm_slli_epi64( xmm_sLTP_shp_Q14_x3x1, 16 );

            xmm_sLTP_shp_Q14_x2x0 = _mm_blend_epi16( xmm_sLTP_shp_Q14_x2x0, xmm_sLTP_shp_Q14_x3x1, 0xCC );

            _mm_storeu_si128( (__m128i *)(&(NSQ->sLTP_shp_Q14[ i ] ) ), xmm_sLTP_shp_Q14_x2x0 );
        }

        for( ; i < NSQ->sLTP_shp_buf_idx; i++ ) {
            NSQ->sLTP_shp_Q14[ i ] = silk_SMULWW( gain_adj_Q16, NSQ->sLTP_shp_Q14[ i ] );
        }

        if( signal_type == TYPE_VOICED && NSQ->rewhite_flag == 0 ) {
            for( i = NSQ->sLTP_buf_idx - lag - LTP_ORDER / 2; i < NSQ->sLTP_buf_idx; i++ ) {
                sLTP_Q15[ i ] = silk_SMULWW( gain_adj_Q16, sLTP_Q15[ i ] );
            }
        }

        NSQ->sLF_AR_shp_Q14 = silk_SMULWW( gain_adj_Q16, NSQ->sLF_AR_shp_Q14 );

        for( i = 0; i < NSQ_LPC_BUF_LENGTH; i++ ) {
            NSQ->sLPC_Q14[ i ] = silk_SMULWW( gain_adj_Q16, NSQ->sLPC_Q14[ i ] );
        }
        for( i = 0; i < MAX_SHAPE_LPC_ORDER; i++ ) {
            NSQ->sAR2_Q14[ i ] = silk_SMULWW( gain_adj_Q16, NSQ->sAR2_Q14[ i ] );
        }
    }
}
