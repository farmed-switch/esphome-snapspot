

#include "pv_audio_type_defs.h"
#include "e_tns_const.h"
#include "tns_decode_coef.h"
#include "fxp_mul32.h"

#define MASK_LOW16  0xffff
#define UPPER16     16

const Int32 tns_table[2][16] =
{
    {
        -2114858546,  -1859775393,  -1380375881,  -734482665,
        0,    931758235,   1678970324,  2093641749
    },
    {
        -2138322861,  -2065504841,  -1922348530,  -1713728946,
        -1446750378,  -1130504462,   -775760571,   -394599085,
        0,    446486956,    873460290,   1262259218,
        1595891361,   1859775393,   2042378317,   2135719508
    }
};

const Int neg_offset[2] = {4, 8};

Int tns_decode_coef(
    const Int   order,
    const Int   coef_res,
    Int32 lpc_coef[TNS_MAX_ORDER],
    Int32 scratchTnsDecCoefMem[2*TNS_MAX_ORDER])
{

    Int i;
    Int m;

    Int32 *pB = &(scratchTnsDecCoefMem[TNS_MAX_ORDER]);

    Int32 *pA = scratchTnsDecCoefMem;

    Int32 *temp_ptr = NULL;

    Int32 *pLPC;
    Int q_lpc = Q_LPC;

    const Int32 *pTnsTable;
    Int coef_offset;
    Int32 table_index;
    Int shift_amount;
    Int32 sin_result;

    Int32 tempInt32;

    Int32 max;
    Int32 mask;

    Int32 mult_high;

    coef_offset = neg_offset[coef_res];
    pTnsTable   = tns_table[coef_res];

    m = 0;
    pLPC = lpc_coef;

    do
    {
        table_index = coef_offset + *(pLPC++);

        sin_result = *(pTnsTable + table_index);

        for (i = m; i > 0; i--)
        {

            mult_high = fxp_mul32_Q31(*(temp_ptr--), sin_result);

            *(pB++) =  *(pA++) + (mult_high << 1);

        }

        *pB =  sin_result >> 12;

        temp_ptr = pA;
        pA       = pB;
        pB       = temp_ptr;

        temp_ptr = pA;

        tempInt32 = *(pA);

        mask = tempInt32 >> 31;
        tempInt32 ^= mask;

        max = tempInt32;

        for (i = m; i > 0; i--)
        {
            tempInt32 = *(--pA);

            mask = tempInt32 >> 31;
            tempInt32 ^= mask;

            max |= tempInt32;
        }

        pB -= m;

        if (max >= 0x40000000L)
        {
            max >>= 1;

            for (i = m; i > 0; i--)
            {
                *(pA++) >>= 1;
                *(pB++) >>= 1;
            }

            *(pA) >>= 1;

            q_lpc--;

            pA -= m;
            pB -= m;
        }

        m++;

    }
    while (m < order);

    shift_amount = 0;

    while (max > 32767)
    {
        max >>= 1;
        shift_amount++;
    }

    if (max != 0)
    {
        while (max < 16384)
        {
            max <<= 1;
            shift_amount--;
        }
    }

    pLPC = lpc_coef;

    if (shift_amount >= 0)
    {

        for (m = order; m > 0; m--)
        {
            *(pLPC++) = *(pA++) << (16 - shift_amount);
        }
    }

    q_lpc -= shift_amount;

    if (q_lpc > 15)
    {
        shift_amount = q_lpc - 15;
        pLPC = lpc_coef;

        for (m = order; m > 0; m--)
        {
            *(pLPC++) >>= shift_amount;
        }

        q_lpc -= shift_amount;
    }

    return (q_lpc);

}
