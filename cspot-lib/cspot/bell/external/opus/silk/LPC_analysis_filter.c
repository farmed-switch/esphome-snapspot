

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "SigProc_FIX.h"
#include "celt_lpc.h"

#define USE_CELT_FIR 0

void silk_LPC_analysis_filter(
    opus_int16                  *out,
    const opus_int16            *in,
    const opus_int16            *B,
    const opus_int32            len,
    const opus_int32            d,
    int                         arch
)
{
    opus_int   j;
#if defined(FIXED_POINT) && USE_CELT_FIR
    opus_int16 num[SILK_MAX_ORDER_LPC];
#else
    int ix;
    opus_int32       out32_Q12, out32;
    const opus_int16 *in_ptr;
#endif

    celt_assert( d >= 6 );
    celt_assert( (d & 1) == 0 );
    celt_assert( d <= len );

#if defined(FIXED_POINT) && USE_CELT_FIR
    celt_assert( d <= SILK_MAX_ORDER_LPC );
    for ( j = 0; j < d; j++ ) {
        num[ j ] = -B[ j ];
    }
    celt_fir( in + d, num, out + d, len - d, d, arch );
    for ( j = 0; j < d; j++ ) {
        out[ j ] = 0;
    }
#else
    (void)arch;
    for( ix = d; ix < len; ix++ ) {
        in_ptr = &in[ ix - 1 ];

        out32_Q12 = silk_SMULBB( in_ptr[  0 ], B[ 0 ] );

        out32_Q12 = silk_SMLABB_ovflw( out32_Q12, in_ptr[ -1 ], B[ 1 ] );
        out32_Q12 = silk_SMLABB_ovflw( out32_Q12, in_ptr[ -2 ], B[ 2 ] );
        out32_Q12 = silk_SMLABB_ovflw( out32_Q12, in_ptr[ -3 ], B[ 3 ] );
        out32_Q12 = silk_SMLABB_ovflw( out32_Q12, in_ptr[ -4 ], B[ 4 ] );
        out32_Q12 = silk_SMLABB_ovflw( out32_Q12, in_ptr[ -5 ], B[ 5 ] );
        for( j = 6; j < d; j += 2 ) {
            out32_Q12 = silk_SMLABB_ovflw( out32_Q12, in_ptr[ -j     ], B[ j     ] );
            out32_Q12 = silk_SMLABB_ovflw( out32_Q12, in_ptr[ -j - 1 ], B[ j + 1 ] );
        }

        out32_Q12 = silk_SUB32_ovflw( silk_LSHIFT( (opus_int32)in_ptr[ 1 ], 12 ), out32_Q12 );

        out32 = silk_RSHIFT_ROUND( out32_Q12, 12 );

        out[ ix ] = (opus_int16)silk_SAT16( out32 );
    }

    silk_memset( out, 0, d * sizeof( opus_int16 ) );
#endif
}
