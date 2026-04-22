

#include "fft_rx4.h"
#include "mix_radix_fft.h"
#include "pv_normalize.h"

#include "fxp_mul32.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void digit_reversal_swapping(Int32 *y, Int32 *x);

#ifdef __cplusplus
}
#endif

Int mix_radix_fft(
    Int32   *Data,
    Int32   *peak_value
)

{

    const Int32     *p_w;
    Int32   *pData_1;
    Int32   *pData_2;

    Int32   *pData_3;
    Int32   *pData_4;

    Int32   exp_jw;
    Int32   max1;
    Int32   max2;
    Int32   temp1;
    Int32   temp2;
    Int32   temp3;
    Int32   temp4;
    Int32   diff1;
    Int32   diff2;
    Int     i;
    Int   exp;

    max1 = *peak_value;
    p_w  = w_512rx2;

    pData_1 = Data;
    pData_3 = Data + HALF_FFT_RX4_LENGTH_FOR_LONG;

    exp = 8 - pv_normalize(max1);

    if (exp < 4)
    {
        exp = 4;
    }

    temp1      = (*pData_3);
    pData_4    = pData_3 + FFT_RX4_LENGTH_FOR_LONG;
    temp2      = (*pData_4++);

    diff1      = (temp1  - temp2) >> exp;
    *pData_3++ = (temp1  + temp2) >> exp;

    temp3      = (*pData_3);
    temp4      = (*pData_4);

    *pData_4-- = -diff1;
    *pData_3++ = (temp3  + temp4) >> exp;
    *pData_4   = (temp3  - temp4) >> exp;

    temp1      = (*pData_1);
    pData_2    = pData_1 + FFT_RX4_LENGTH_FOR_LONG;
    temp2      = (*pData_2++);
    temp4      = (*pData_2);

    *pData_1++ = (temp1  + temp2) >> exp;

    temp3      = (*pData_1);
    diff1      = (temp1  - temp2) >> exp ;

    *pData_1++ = (temp3  + temp4) >> exp;
    *pData_2-- = (temp3  - temp4) >> exp;
    *pData_2   =  diff1;

    temp1      = (*pData_3);
    pData_4    = pData_3 + FFT_RX4_LENGTH_FOR_LONG;
    temp2      = (*pData_4++);

    for (i = ONE_FOURTH_FFT_RX4_LENGTH_FOR_LONG - 1; i != 0; i--)
    {

        diff1      = (temp1  - temp2) >> (exp - 4);
        *pData_3++ = (temp1  + temp2) >> exp;

        temp3      = (*pData_3);
        temp4      = (*pData_4);

        exp_jw     = *p_w++;

        diff2      = (temp3  - temp4) >> (exp - 4);
        *pData_3++ = (temp3  + temp4) >> exp;

        *pData_4-- = -cmplx_mul32_by_16(diff1,  diff2, exp_jw) >> 3;
        *pData_4   =  cmplx_mul32_by_16(diff2, -diff1, exp_jw) >> 3;

        temp1      = (*pData_1);
        pData_2    = pData_1 + FFT_RX4_LENGTH_FOR_LONG;
        temp2      = (*pData_2++);
        temp4      = (*pData_2);

        *pData_1++ = (temp1  + temp2) >> exp;

        temp3      = (*pData_1);
        diff1      = (temp1  - temp2) >> (exp - 4);

        diff2      = (temp3  - temp4) >> (exp - 4);
        *pData_1++ = (temp3  + temp4) >> exp;

        *pData_2-- =  cmplx_mul32_by_16(diff2, -diff1, exp_jw) >> 3;
        *pData_2   =  cmplx_mul32_by_16(diff1,  diff2, exp_jw) >> 3;

        temp1      = (*pData_3);
        pData_4    = pData_3 + FFT_RX4_LENGTH_FOR_LONG;
        temp2      = (*pData_4++);

    }

    fft_rx4_long(
        Data,
        &max1);

    fft_rx4_long(
        &Data[FFT_RX4_LENGTH_FOR_LONG],
        &max2);

    digit_reversal_swapping(Data, &Data[FFT_RX4_LENGTH_FOR_LONG]);

    *peak_value = max1 | max2;

    return(exp);
}

