

#include "pv_audio_type_defs.h"
#include "tns_inv_filter.h"
#include "fxp_mul32.h"

void tns_inv_filter(
    Int32 coef[],
    const Int num_coef,
    const Int direction,
    const Int32 lpc[],
    const Int lpc_qformat,
    const Int order,
    Int32 scratch_memory[])
{

    Int i;
    Int j;
    Int shift_amt;
    Int wrap_point;

    Int32 mult;

    Int32 *pFilterInput = scratch_memory;

    const Int32 *pLPC;

    Int32 *pCoef = coef;

    if (direction == -1)
    {
        pCoef += (num_coef - 1);
    }

    for (i = order; i != 0; i--)
    {
        *(pFilterInput++) = 0;
    }

    wrap_point = 0;

    shift_amt  = (lpc_qformat - 5);

    for (i = num_coef; i > 0; i--)
    {

        pLPC = lpc;

        mult = 0;

        for (j = wrap_point; j > 0; j--)
        {
            mult += fxp_mul32_Q31(*(pLPC++), *(pFilterInput++)) >> 5;

        }

        pFilterInput = scratch_memory;

        for (j = (order - wrap_point); j > 0; j--)
        {
            mult += fxp_mul32_Q31(*(pLPC++), *(pFilterInput++)) >> 5;

        }

        *(--pFilterInput) = (*pCoef);

        *(pCoef) += (mult >> shift_amt);

        pCoef += direction;

        wrap_point++;

        if (wrap_point == order)
        {
            wrap_point = 0;
        }

    }

}
