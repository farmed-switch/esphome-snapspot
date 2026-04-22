

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "SigProc_FIX.h"
#include "tables.h"

#define QA      16

static OPUS_INLINE void silk_NLSF2A_find_poly(
    opus_int32          *out,
    const opus_int32    *cLSF,
    opus_int            dd
)
{
    opus_int   k, n;
    opus_int32 ftmp;

    out[0] = silk_LSHIFT( 1, QA );
    out[1] = -cLSF[0];
    for( k = 1; k < dd; k++ ) {
        ftmp = cLSF[2*k];
        out[k+1] = silk_LSHIFT( out[k-1], 1 ) - (opus_int32)silk_RSHIFT_ROUND64( silk_SMULL( ftmp, out[k] ), QA );
        for( n = k; n > 1; n-- ) {
            out[n] += out[n-2] - (opus_int32)silk_RSHIFT_ROUND64( silk_SMULL( ftmp, out[n-1] ), QA );
        }
        out[1] -= ftmp;
    }
}

void silk_NLSF2A(
    opus_int16                  *a_Q12,
    const opus_int16            *NLSF,
    const opus_int              d,
    int                         arch
)
{

    static const unsigned char ordering16[16] = {
      0, 15, 8, 7, 4, 11, 12, 3, 2, 13, 10, 5, 6, 9, 14, 1
    };
    static const unsigned char ordering10[10] = {
      0, 9, 6, 3, 4, 5, 8, 1, 2, 7
    };
    const unsigned char *ordering;
    opus_int   k, i, dd;
    opus_int32 cos_LSF_QA[ SILK_MAX_ORDER_LPC ];
    opus_int32 P[ SILK_MAX_ORDER_LPC / 2 + 1 ], Q[ SILK_MAX_ORDER_LPC / 2 + 1 ];
    opus_int32 Ptmp, Qtmp, f_int, f_frac, cos_val, delta;
    opus_int32 a32_QA1[ SILK_MAX_ORDER_LPC ];

    silk_assert( LSF_COS_TAB_SZ_FIX == 128 );
    celt_assert( d==10 || d==16 );

    ordering = d == 16 ? ordering16 : ordering10;
    for( k = 0; k < d; k++ ) {
        silk_assert( NLSF[k] >= 0 );

        f_int = silk_RSHIFT( NLSF[k], 15 - 7 );

        f_frac = NLSF[k] - silk_LSHIFT( f_int, 15 - 7 );

        silk_assert(f_int >= 0);
        silk_assert(f_int < LSF_COS_TAB_SZ_FIX );

        cos_val = silk_LSFCosTab_FIX_Q12[ f_int ];
        delta   = silk_LSFCosTab_FIX_Q12[ f_int + 1 ] - cos_val;

        cos_LSF_QA[ordering[k]] = silk_RSHIFT_ROUND( silk_LSHIFT( cos_val, 8 ) + silk_MUL( delta, f_frac ), 20 - QA );
    }

    dd = silk_RSHIFT( d, 1 );

    silk_NLSF2A_find_poly( P, &cos_LSF_QA[ 0 ], dd );
    silk_NLSF2A_find_poly( Q, &cos_LSF_QA[ 1 ], dd );

    for( k = 0; k < dd; k++ ) {
        Ptmp = P[ k+1 ] + P[ k ];
        Qtmp = Q[ k+1 ] - Q[ k ];

        a32_QA1[ k ]     = -Qtmp - Ptmp;
        a32_QA1[ d-k-1 ] =  Qtmp - Ptmp;
    }

    silk_LPC_fit( a_Q12, a32_QA1, 12, QA + 1, d );

    for( i = 0; silk_LPC_inverse_pred_gain( a_Q12, d, arch ) == 0 && i < MAX_LPC_STABILIZE_ITERATIONS; i++ ) {

        silk_bwexpander_32( a32_QA1, d, 65536 - silk_LSHIFT( 2, i ) );
        for( k = 0; k < d; k++ ) {
            a_Q12[ k ] = (opus_int16)silk_RSHIFT_ROUND( a32_QA1[ k ], QA + 1 - 12 );
        }
    }
}

