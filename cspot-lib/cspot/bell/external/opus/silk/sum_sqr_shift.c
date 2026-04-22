

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "SigProc_FIX.h"

void silk_sum_sqr_shift(
    opus_int32                  *energy,
    opus_int                    *shift,
    const opus_int16            *x,
    opus_int                    len
)
{
    opus_int   i, shft;
    opus_uint32 nrg_tmp;
    opus_int32 nrg;

    shft = 31-silk_CLZ32(len);

    nrg  = len;
    for( i = 0; i < len - 1; i += 2 ) {
        nrg_tmp = silk_SMULBB( x[ i ], x[ i ] );
        nrg_tmp = silk_SMLABB_ovflw( nrg_tmp, x[ i + 1 ], x[ i + 1 ] );
        nrg = (opus_int32)silk_ADD_RSHIFT_uint( nrg, nrg_tmp, shft );
    }
    if( i < len ) {

        nrg_tmp = silk_SMULBB( x[ i ], x[ i ] );
        nrg = (opus_int32)silk_ADD_RSHIFT_uint( nrg, nrg_tmp, shft );
    }
    silk_assert( nrg >= 0 );

    shft = silk_max_32(0, shft+3 - silk_CLZ32(nrg));
    nrg = 0;
    for( i = 0 ; i < len - 1; i += 2 ) {
        nrg_tmp = silk_SMULBB( x[ i ], x[ i ] );
        nrg_tmp = silk_SMLABB_ovflw( nrg_tmp, x[ i + 1 ], x[ i + 1 ] );
        nrg = (opus_int32)silk_ADD_RSHIFT_uint( nrg, nrg_tmp, shft );
    }
    if( i < len ) {

        nrg_tmp = silk_SMULBB( x[ i ], x[ i ] );
        nrg = (opus_int32)silk_ADD_RSHIFT_uint( nrg, nrg_tmp, shft );
    }

    silk_assert( nrg >= 0 );

    *shift  = shft;
    *energy = nrg;
}

