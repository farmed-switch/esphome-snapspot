

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main_FIX.h"

void silk_corrVector_FIX(
    const opus_int16                *x,
    const opus_int16                *t,
    const opus_int                  L,
    const opus_int                  order,
    opus_int32                      *Xt,
    const opus_int                  rshifts,
    int                             arch
)
{
    opus_int         lag, i;
    const opus_int16 *ptr1, *ptr2;
    opus_int32       inner_prod;

    ptr1 = &x[ order - 1 ];
    ptr2 = t;

    if( rshifts > 0 ) {

        for( lag = 0; lag < order; lag++ ) {
            inner_prod = 0;
            for( i = 0; i < L; i++ ) {
                inner_prod = silk_ADD_RSHIFT32( inner_prod, silk_SMULBB( ptr1[ i ], ptr2[i] ), rshifts );
            }
            Xt[ lag ] = inner_prod;
            ptr1--;
        }
    } else {
        silk_assert( rshifts == 0 );
        for( lag = 0; lag < order; lag++ ) {
            Xt[ lag ] = silk_inner_prod_aligned( ptr1, ptr2, L, arch );
            ptr1--;
        }
    }
}

void silk_corrMatrix_FIX(
    const opus_int16                *x,
    const opus_int                  L,
    const opus_int                  order,
    opus_int32                      *XX,
    opus_int32                      *nrg,
    opus_int                        *rshifts,
    int                             arch
)
{
    opus_int         i, j, lag;
    opus_int32       energy;
    const opus_int16 *ptr1, *ptr2;

    silk_sum_sqr_shift( nrg, rshifts, x, L + order - 1 );
    energy = *nrg;

    for( i = 0; i < order - 1; i++ ) {
        energy -= silk_RSHIFT32( silk_SMULBB( x[ i ], x[ i ] ), *rshifts );
    }

    matrix_ptr( XX, 0, 0, order ) = energy;
    silk_assert( energy >= 0 );
    ptr1 = &x[ order - 1 ];
    for( j = 1; j < order; j++ ) {
        energy = silk_SUB32( energy, silk_RSHIFT32( silk_SMULBB( ptr1[ L - j ], ptr1[ L - j ] ), *rshifts ) );
        energy = silk_ADD32( energy, silk_RSHIFT32( silk_SMULBB( ptr1[ -j ], ptr1[ -j ] ), *rshifts ) );
        matrix_ptr( XX, j, j, order ) = energy;
        silk_assert( energy >= 0 );
    }

    ptr2 = &x[ order - 2 ];

    if( *rshifts > 0 ) {

        for( lag = 1; lag < order; lag++ ) {

            energy = 0;
            for( i = 0; i < L; i++ ) {
                energy += silk_RSHIFT32( silk_SMULBB( ptr1[ i ], ptr2[i] ), *rshifts );
            }

            matrix_ptr( XX, lag, 0, order ) = energy;
            matrix_ptr( XX, 0, lag, order ) = energy;
            for( j = 1; j < ( order - lag ); j++ ) {
                energy = silk_SUB32( energy, silk_RSHIFT32( silk_SMULBB( ptr1[ L - j ], ptr2[ L - j ] ), *rshifts ) );
                energy = silk_ADD32( energy, silk_RSHIFT32( silk_SMULBB( ptr1[ -j ], ptr2[ -j ] ), *rshifts ) );
                matrix_ptr( XX, lag + j, j, order ) = energy;
                matrix_ptr( XX, j, lag + j, order ) = energy;
            }
            ptr2--;
        }
    } else {
        for( lag = 1; lag < order; lag++ ) {

            energy = silk_inner_prod_aligned( ptr1, ptr2, L, arch );
            matrix_ptr( XX, lag, 0, order ) = energy;
            matrix_ptr( XX, 0, lag, order ) = energy;

            for( j = 1; j < ( order - lag ); j++ ) {
                energy = silk_SUB32( energy, silk_SMULBB( ptr1[ L - j ], ptr2[ L - j ] ) );
                energy = silk_SMLABB( energy, ptr1[ -j ], ptr2[ -j ] );
                matrix_ptr( XX, lag + j, j, order ) = energy;
                matrix_ptr( XX, j, lag + j, order ) = energy;
            }
            ptr2--;
        }
    }
}

