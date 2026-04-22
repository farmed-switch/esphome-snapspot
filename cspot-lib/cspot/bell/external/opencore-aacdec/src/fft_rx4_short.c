

#include "pv_audio_type_defs.h"
#include "fft_rx4.h"
#include "pv_normalize.h"
#include "fxp_mul32.h"

Int fft_rx4_short(
    Int32      Data[],
    Int32      *peak_value)

{
    Int     n1;
    Int     n2;
    Int     n3;
    Int     j;
    Int     k;
    Int     i;
    Int32   exp_jw1;
    Int32   exp_jw2;
    Int32   exp_jw3;

    Int32   t1;
    Int32   t2;
    Int32   r1;
    Int32   r2;
    Int32   r3;
    Int32   s1;
    Int32   s2;
    Int32   s3;

    Int32   *pData1;
    Int32   *pData2;
    Int32   *pData3;
    Int32   *pData4;
    const Int32  *pw;
    Int32   temp1;
    Int32   temp2;
    Int32   temp3;
    Int32   temp4;
    Int32   max;
    Int     exp;
    Int     exponent = 0;
    Int     shift;

    max = *peak_value;
    exp = 0;

    if (max > 0x008000)
    {
        exp = 8 - pv_normalize(max);

        exponent = exp;

    }

    n2 = FFT_RX4_SHORT;

    pw = W_64rx4;

    shift = 2;

    for (k = FFT_RX4_SHORT; k > 4; k >>= 2)
    {

        n1 = n2;
        n2 >>= 2;
        n3 = n1 >> 1;

        exp -= 2;

        for (i = 0; i < FFT_RX4_SHORT; i += n1)
        {
            pData1 = &Data[ i<<1];
            pData3 = pData1 + n3;
            pData2 = pData1 + n1;
            pData4 = pData3 + n1;

            temp1   = *(pData1);
            temp2   = *(pData2);
            temp1   >>= shift;
            temp2   >>= shift;

            r1      = temp1 + temp2;
            r2      = temp1 - temp2;

            temp3   = *(pData3++);
            temp4   = *(pData4++);
            temp3   >>= shift;
            temp4   >>= shift;

            t1      = temp3 + temp4;
            t2      = temp3 - temp4;

            *(pData1++) = (r1 + t1) >> exp;
            *(pData2++) = (r1 - t1) >> exp;

            temp1   = *pData1;
            temp2   = *pData2;
            temp1   >>= shift;
            temp2   >>= shift;

            s1      = temp1 + temp2;
            s2      = temp1 - temp2;

            temp3   = *pData3;
            temp4   = *pData4;
            temp3   >>= shift;
            temp4   >>= shift;

            t1      = temp3 + temp4;
            r1      = temp3 - temp4;

            *pData1   = (s1 + t1) >> exp;
            *pData2   = (s1 - t1) >> exp;

            *pData4--    = (s2 + t2) >> exp;
            *pData4      = (r2 - r1) >> exp;

            *pData3--    = (s2 - t2) >> exp;
            *pData3      = (r2 + r1) >> exp;

        }

        for (j = 1; j < n2; j++)
        {
            exp_jw1 = *pw++;
            exp_jw2 = *pw++;
            exp_jw3 = *pw++;

            for (i = j; i < FFT_RX4_SHORT; i += n1)
            {
                pData1 = &Data[ i<<1];
                pData3 = pData1 + n3;
                pData2 = pData1 + n1;
                pData4 = pData3 + n1;

                temp1   = *(pData1);
                temp2   = *(pData2++);
                temp1   >>= shift;
                temp2   >>= shift;

                r1      = temp1 + temp2;
                r2      = temp1 - temp2;
                temp3   = *(pData3++);
                temp4   = *(pData4++);
                temp3   >>= shift;
                temp4   >>= shift;

                t1      = temp3 + temp4;
                t2      = temp3 - temp4;

                *(pData1++) = (r1 + t1) >> exp;
                r1          = (r1 - t1) >> exp;

                temp1   = *pData1;
                temp2   = *pData2;
                temp1   >>= shift;
                temp2   >>= shift;

                s1      = temp1 + temp2;
                s2      = temp1 - temp2;

                s3      = (s2 + t2) >> exp;
                s2      = (s2 - t2) >> exp;

                temp3   = *pData3;
                temp4   = *pData4 ;
                temp3   >>= shift;
                temp4   >>= shift;

                t1      = temp3 + temp4;
                t2      = temp3 - temp4;

                *pData1  = (s1 + t1) >> exp;
                s1       = (s1 - t1) >> exp;

                *pData2--  = cmplx_mul32_by_16(s1, -r1, exp_jw2) << 1;
                *pData2    = cmplx_mul32_by_16(r1,  s1, exp_jw2) << 1;

                r3       = ((r2 - t2) >> exp);
                r2       = ((r2 + t2) >> exp);

                *pData3--  = cmplx_mul32_by_16(s2, -r2, exp_jw1) << 1;
                *pData3    = cmplx_mul32_by_16(r2,  s2, exp_jw1) << 1;

                *pData4--  = cmplx_mul32_by_16(s3, -r3, exp_jw3) << 1;
                *pData4    = cmplx_mul32_by_16(r3,  s3, exp_jw3) << 1;

            }

        }

        exp   = 2;
        shift = 0;

    }

    max = 0;

    pData1 = Data - 7;

    for (i = ONE_FOURTH_FFT_RX4_SHORT; i != 0 ; i--)
    {
        pData1 += 7;

        pData3 = pData1 + 2;
        pData2 = pData1 + 4;
        pData4 = pData1 + 6;

        temp1   = *pData1;
        temp2   = *pData2++;

        r1      = temp1 + temp2;
        r2      = temp1 - temp2;

        temp1   = *pData3++;
        temp2   = *pData4++;

        t1      = temp1 + temp2;
        t2      = temp1 - temp2;

        temp1       = (r1 + t1);
        r1          = (r1 - t1);
        *(pData1++) = temp1;
        max        |= (temp1 >> 31) ^ temp1;

        temp1   = *pData1;
        temp2   = *pData2;

        s1      = temp1 + temp2;
        s2      = temp1 - temp2;

        s3      = (s2 + t2);
        s2      = (s2 - t2);

        temp1   = *pData3;
        temp2   = *pData4;

        t1      = temp1 + temp2;
        t2      = temp1 - temp2;

        temp1      = (s1 + t1);
        temp2      = (s1 - t1);
        *pData1    = temp1;
        *pData2--  = temp2;
        max       |= (temp1 >> 31) ^ temp1;
        max       |= (temp2 >> 31) ^ temp2;

        *pData2    = r1;
        *pData3--  = s2;
        *pData4--  = s3;
        max       |= (r1 >> 31) ^ r1;
        max       |= (s2 >> 31) ^ s2;
        max       |= (s3 >> 31) ^ s3;

        temp1      = (r2 - t2);
        temp2      = (r2 + t2);
        *pData4    = temp1;
        *pData3    = temp2;
        max       |= (temp1 >> 31) ^ temp1;
        max       |= (temp2 >> 31) ^ temp2;

    }

    *peak_value = max;

    return (exponent);

}
