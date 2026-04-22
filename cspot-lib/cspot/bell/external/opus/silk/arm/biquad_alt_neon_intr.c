

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <arm_neon.h>
#ifdef OPUS_CHECK_ASM
# include <string.h>
# include "stack_alloc.h"
#endif
#include "SigProc_FIX.h"

static inline void silk_biquad_alt_stride2_kernel( const int32x4_t A_L_s32x4, const int32x4_t A_U_s32x4, const int32x4_t B_Q28_s32x4, const int32x2_t t_s32x2, const int32x4_t in_s32x4, int32x4_t *S_s32x4, int32x2_t *out32_Q14_s32x2 )
{
    int32x4_t t_s32x4, out32_Q14_s32x4;

    *out32_Q14_s32x2 = vadd_s32( vget_low_s32( *S_s32x4 ), t_s32x2 );
    *S_s32x4         = vcombine_s32( vget_high_s32( *S_s32x4 ), vdup_n_s32( 0 ) );
    *out32_Q14_s32x2 = vshl_n_s32( *out32_Q14_s32x2, 2 );
    out32_Q14_s32x4  = vcombine_s32( *out32_Q14_s32x2, *out32_Q14_s32x2 );
    t_s32x4          = vqdmulhq_s32( out32_Q14_s32x4, A_L_s32x4 );
    *S_s32x4         = vrsraq_n_s32( *S_s32x4, t_s32x4, 14 );
    t_s32x4          = vqdmulhq_s32( out32_Q14_s32x4, A_U_s32x4 );
    *S_s32x4         = vaddq_s32( *S_s32x4, t_s32x4 );
    t_s32x4          = vqdmulhq_s32( in_s32x4, B_Q28_s32x4 );
    *S_s32x4         = vaddq_s32( *S_s32x4, t_s32x4 );
}

void silk_biquad_alt_stride2_neon(
    const opus_int16            *in,
    const opus_int32            *B_Q28,
    const opus_int32            *A_Q28,
    opus_int32                  *S,
    opus_int16                  *out,
    const opus_int32            len
)
{

    opus_int        k            = 0;
    const int32x2_t offset_s32x2 = vdup_n_s32( (1<<14) - 1 );
    const int32x4_t offset_s32x4 = vcombine_s32( offset_s32x2, offset_s32x2 );
    int16x4_t       in_s16x4  = vdup_n_s16( 0 );
    int16x4_t       out_s16x4;
    int32x2_t       A_Q28_s32x2, A_L_s32x2, A_U_s32x2, B_Q28_s32x2, t_s32x2;
    int32x4_t       A_L_s32x4, A_U_s32x4, B_Q28_s32x4, S_s32x4, out32_Q14_s32x4;
    int32x2x2_t     t0_s32x2x2, t1_s32x2x2, t2_s32x2x2, S_s32x2x2;

#ifdef OPUS_CHECK_ASM
    opus_int32 S_c[ 4 ];
    VARDECL( opus_int16, out_c );
    SAVE_STACK;
    ALLOC( out_c, 2 * len, opus_int16 );

    silk_memcpy( &S_c, S, sizeof( S_c ) );
    silk_biquad_alt_stride2_c( in, B_Q28, A_Q28, S_c, out_c, len );
#endif

    A_Q28_s32x2 = vld1_s32( A_Q28 );
    A_Q28_s32x2 = vneg_s32( A_Q28_s32x2 );
    A_L_s32x2   = vshl_n_s32( A_Q28_s32x2, 18 );
    A_L_s32x2   = vreinterpret_s32_u32( vshr_n_u32( vreinterpret_u32_s32( A_L_s32x2 ), 3 ) );
    A_U_s32x2   = vshr_n_s32( A_Q28_s32x2, 14 );
    A_U_s32x2   = vshl_n_s32( A_U_s32x2, 16 );
    A_U_s32x2   = vshr_n_s32( A_U_s32x2, 1 );

    B_Q28_s32x2  = vld1_s32( B_Q28 );
    t_s32x2      = vld1_s32( B_Q28 + 1 );
    t0_s32x2x2   = vzip_s32( A_L_s32x2, A_L_s32x2 );
    t1_s32x2x2   = vzip_s32( A_U_s32x2, A_U_s32x2 );
    t2_s32x2x2   = vzip_s32( t_s32x2, t_s32x2 );
    A_L_s32x4    = vcombine_s32( t0_s32x2x2.val[ 0 ], t0_s32x2x2.val[ 1 ] );
    A_U_s32x4    = vcombine_s32( t1_s32x2x2.val[ 0 ], t1_s32x2x2.val[ 1 ] );
    B_Q28_s32x4  = vcombine_s32( t2_s32x2x2.val[ 0 ], t2_s32x2x2.val[ 1 ] );
    S_s32x4      = vld1q_s32( S );
    S_s32x2x2    = vtrn_s32( vget_low_s32( S_s32x4 ), vget_high_s32( S_s32x4 ) );
    S_s32x4      = vcombine_s32( S_s32x2x2.val[ 0 ], S_s32x2x2.val[ 1 ] );

    for( ; k < len - 1; k += 2 ) {
        int32x4_t in_s32x4[ 2 ], t_s32x4;
        int32x2_t out32_Q14_s32x2[ 2 ];

        in_s16x4      = vld1_s16( &in[ 2 * k ] );
        in_s32x4[ 0 ] = vshll_n_s16( in_s16x4, 15 );
        t_s32x4       = vqdmulhq_lane_s32( in_s32x4[ 0 ], B_Q28_s32x2, 0 );
        in_s32x4[ 1 ] = vcombine_s32( vget_high_s32( in_s32x4[ 0 ] ), vget_high_s32( in_s32x4[ 0 ] ) );
        in_s32x4[ 0 ] = vcombine_s32( vget_low_s32 ( in_s32x4[ 0 ] ), vget_low_s32 ( in_s32x4[ 0 ] ) );
        silk_biquad_alt_stride2_kernel( A_L_s32x4, A_U_s32x4, B_Q28_s32x4, vget_low_s32 ( t_s32x4 ), in_s32x4[ 0 ], &S_s32x4, &out32_Q14_s32x2[ 0 ] );
        silk_biquad_alt_stride2_kernel( A_L_s32x4, A_U_s32x4, B_Q28_s32x4, vget_high_s32( t_s32x4 ), in_s32x4[ 1 ], &S_s32x4, &out32_Q14_s32x2[ 1 ] );

        out32_Q14_s32x4 = vcombine_s32( out32_Q14_s32x2[ 0 ], out32_Q14_s32x2[ 1 ] );
        out32_Q14_s32x4 = vaddq_s32( out32_Q14_s32x4, offset_s32x4 );
        out_s16x4       = vqshrn_n_s32( out32_Q14_s32x4, 14 );
        vst1_s16( &out[ 2 * k ], out_s16x4 );
    }

    if( k < len ) {
        int32x4_t in_s32x4;
        int32x2_t out32_Q14_s32x2;

        in_s16x4     = vld1_lane_s16( &in[ 2 * k + 0 ], in_s16x4, 0 );
        in_s16x4     = vld1_lane_s16( &in[ 2 * k + 1 ], in_s16x4, 1 );
        in_s32x4     = vshll_n_s16( in_s16x4, 15 );
        t_s32x2      = vqdmulh_lane_s32( vget_low_s32( in_s32x4 ), B_Q28_s32x2, 0 );
        in_s32x4     = vcombine_s32( vget_low_s32( in_s32x4 ), vget_low_s32( in_s32x4 ) );
        silk_biquad_alt_stride2_kernel( A_L_s32x4, A_U_s32x4, B_Q28_s32x4, t_s32x2, in_s32x4, &S_s32x4, &out32_Q14_s32x2 );

        out32_Q14_s32x2 = vadd_s32( out32_Q14_s32x2, offset_s32x2 );
        out32_Q14_s32x4 = vcombine_s32( out32_Q14_s32x2, out32_Q14_s32x2 );
        out_s16x4       = vqshrn_n_s32( out32_Q14_s32x4, 14 );
        vst1_lane_s16( &out[ 2 * k + 0 ], out_s16x4, 0 );
        vst1_lane_s16( &out[ 2 * k + 1 ], out_s16x4, 1 );
    }

    vst1q_lane_s32( &S[ 0 ], S_s32x4, 0 );
    vst1q_lane_s32( &S[ 1 ], S_s32x4, 2 );
    vst1q_lane_s32( &S[ 2 ], S_s32x4, 1 );
    vst1q_lane_s32( &S[ 3 ], S_s32x4, 3 );

#ifdef OPUS_CHECK_ASM
    silk_assert( !memcmp( S_c, S, sizeof( S_c ) ) );
    silk_assert( !memcmp( out_c, out, 2 * len * sizeof( opus_int16 ) ) );
    RESTORE_STACK;
#endif
}
