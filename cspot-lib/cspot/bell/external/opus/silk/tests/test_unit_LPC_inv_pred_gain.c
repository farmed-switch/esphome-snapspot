

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include "celt/stack_alloc.h"
#include "cpu_support.h"
#include "SigProc_FIX.h"

int check_stability(opus_int16 *A_Q12, int order) {
    int i;
    int j;
    int sum_a, sum_abs_a;
    sum_a = sum_abs_a = 0;
    for( j = 0; j < order; j++ ) {
        sum_a += A_Q12[ j ];
        sum_abs_a += silk_abs( A_Q12[ j ] );
    }

    if( sum_a >= 4096 ) {
        return 0;
    }

    if( sum_abs_a < 4096 ) {
        return 1;
    }
    double y[SILK_MAX_ORDER_LPC] = {0};
    y[0] = 1;
    for( i = 0; i < 10000; i++ ) {
        double sum = 0;
        for( j = 0; j < order; j++ ) {
            sum += y[ j ]*A_Q12[ j ];
        }
        for( j = order - 1; j > 0; j-- ) {
            y[ j ] = y[ j - 1 ];
        }
        y[ 0 ] = sum*(1./4096);

        if( !(y[ 0 ] < 10000 && y[ 0 ] > -10000) ) {
            return 0;
        }

        if( ( i & 0x7 ) == 0 ) {
            double amp = 0;
            for( j = 0; j < order; j++ ) {
                amp += fabs(y[j]);
            }
            if( amp < 0.00001 ) {
                return 1;
            }
        }
    }
    return 1;
}

int main(void) {
    const int arch = opus_select_arch();

    const int loop_num = 10000;
    int count = 0;
    ALLOC_STACK;

    srand(0);

    printf("Testing silk_LPC_inverse_pred_gain() optimization ...\n");
    for( count = 0; count < loop_num; count++ ) {
        unsigned int i;
        opus_int     order;
        unsigned int shift;
        opus_int16   A_Q12[ SILK_MAX_ORDER_LPC ];
        opus_int32 gain;

        for( order = 2; order <= SILK_MAX_ORDER_LPC; order += 2 ) {
            for( shift = 0; shift < 16; shift++ ) {
                for( i = 0; i < SILK_MAX_ORDER_LPC; i++ ) {
                    A_Q12[i] = ((opus_int16)rand()) >> shift;
                }
                gain = silk_LPC_inverse_pred_gain(A_Q12, order, arch);

                if( gain != 0 && !check_stability(A_Q12, order) ) {
                    fprintf(stderr, "**Loop %4d failed!**\n", count);
                    return 1;
                }
            }
        }
        if( !(count % 500) ) {
            printf("Loop %4d passed\n", count);
        }
    }
    printf("silk_LPC_inverse_pred_gain() optimization passed\n");
    return 0;
}
