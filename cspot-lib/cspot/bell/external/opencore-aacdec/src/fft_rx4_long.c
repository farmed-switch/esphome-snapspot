

#include "pv_audio_type_defs.h"
#include "fft_rx4.h"

#include "fxp_mul32.h"

void fft_rx4_long(
    Int32      Data[],
    Int32      *peak_value)

{
    Int     n1;
    Int     n2;
    Int     j;
    Int     k;
    Int     i;

    Int32   t1;
    Int32   t2;
    Int32   r1;
    Int32   r2;
    Int32   r3;
    Int32   r4;
    Int32   s1;
    Int32   s2;
    Int32   s3;
    Int32   *pData1;
    Int32   *pData2;
    Int32   *pData3;
    Int32   *pData4;
    Int32   temp1;
    Int32   temp2;
    Int32   temp3;
    Int32   temp4;
    Int32   max;

    Int32   exp_jw1;
    Int32   exp_jw2;
    Int32   exp_jw3;

    const Int32  *pw = W_256rx4;

    n2 = FFT_RX4_LONG;

    for (k = FFT_RX4_LONG; k > 4; k >>= 2)
    {

        n1 = n2;
        n2 >>= 2;

        for (i = 0; i < FFT_RX4_LONG; i += n1)
        {
            pData1 = &Data[ i<<1];
            pData2 = pData1 + n1;

            temp1   = *pData1;
            temp2   = *pData2;

            r1      = temp1 + temp2;
            r2      = temp1 - temp2;

            pData3 = pData1 + (n1 >> 1);
            pData4 = pData3 + n1;
            temp3   = *pData3++;
            temp4   = *pData4++;

            t1      = temp3 + temp4;

            *(pData1++) = (r1 + t1);
            t2      = temp3 - temp4;
            *(pData2++) = (r1 - t1);

            temp1   = *pData1;
            temp2   = *pData2;

            s1      = temp1 + temp2;
            temp3   = *pData3;
            s2      = temp1 - temp2;
            temp4   = *pData4;
            *pData3--  = (s2 - t2);
            *pData4--  = (s2 + t2);

            t1      = temp3 + temp4;

            *pData1    = (s1 + t1);
            *pData2    = (s1 - t1);

            r1      = temp3 - temp4;

            *pData4    = (r2 - r1);
            *pData3    = (r2 + r1);

        }

        for (j = 1; j < n2; j++)
        {

            exp_jw1 = (*pw++);
            exp_jw2 = (*pw++);
            exp_jw3 = (*pw++);

            for (i = j; i < FFT_RX4_LONG; i += n1)
            {
                pData1 = &Data[ i<<1];
                pData2 = pData1 + n1;

                temp1   = *pData1;
                temp2   = *pData2++;

                r1      = temp1 + temp2;
                r2      = temp1 - temp2;

                pData3 = pData1 + (n1 >> 1);
                pData4 = pData3 + n1;
                temp3   = *pData3++;
                temp4   = *pData4++;

                r3      = temp3 + temp4;
                r4      = temp3 - temp4;

                *(pData1++) = (r1 + r3);
                r1          = (r1 - r3) << 1;

                temp2   = *pData2;
                temp1   = *pData1;

                s1      = temp1 + temp2;
                s2      = temp1 - temp2;
                s3      = (s2 + r4) << 1;
                s2      = (s2 - r4) << 1;

                temp3   = *pData3;
                temp4   = *pData4;

                t1      = temp3 + temp4;
                t2      = temp3 - temp4;

                *pData1  = (s1 + t1);
                s1       = (s1 - t1) << 1;

                *pData2--  = cmplx_mul32_by_16(s1, -r1, exp_jw2);
                r3      = (r2 - t2) << 1;
                *pData2    = cmplx_mul32_by_16(r1,  s1, exp_jw2);

                r2      = (r2 + t2) << 1;

                *pData3--  = cmplx_mul32_by_16(s2, -r2, exp_jw1);
                *pData3    = cmplx_mul32_by_16(r2,  s2, exp_jw1);

                *pData4--  = cmplx_mul32_by_16(s3, -r3, exp_jw3);
                *pData4    = cmplx_mul32_by_16(r3,  s3, exp_jw3);

            }

        }

    }

    max = 0;

    pData1 = Data - 7;

    for (i = ONE_FOURTH_FFT_RX4_LONG; i != 0 ; i--)
    {
        pData1 += 7;
        pData2 = pData1 + 4;

        temp1   = *pData1;
        temp2   = *pData2++;

        r1      = temp1 + temp2;
        r2      = temp1 - temp2;

        pData3 = pData1 + 2;
        pData4 = pData1 + 6;
        temp1   = *pData3++;
        temp2   = *pData4++;

        t1      = temp1 + temp2;
        t2      = temp1 - temp2;

        temp1       = (r1 + t1);
        r1          = (r1 - t1);
        *(pData1++) = temp1;
        max        |= (temp1 >> 31) ^ temp1;

        temp2   = *pData2;
        temp1   = *pData1;

        s1      = temp1 + temp2;
        s2      = temp1 - temp2;

        temp1   = *pData3;
        temp2   = *pData4;

        s3      = (s2 + t2);
        s2      = (s2 - t2);

        t1      = temp1 + temp2;
        t2      = temp1 - temp2;

        temp1      = (s1 + t1);
        *pData1    = temp1;
        temp2      = (s1 - t1);

        max       |= (temp1 >> 31) ^ temp1;
        *pData2--  = temp2;
        max       |= (temp2 >> 31) ^ temp2;

        *pData2    = r1;
        max       |= (r1 >> 31) ^ r1;
        *pData3--  = s2;
        max       |= (s2 >> 31) ^ s2;
        *pData4--  = s3;
        max       |= (s3 >> 31) ^ s3;

        temp1      = (r2 - t2);
        *pData4    = temp1;
        temp2      = (r2 + t2);
        *pData3    = temp2;
        max       |= (temp1 >> 31) ^ temp1;
        max       |= (temp2 >> 31) ^ temp2;

    }

    *peak_value = max;

    return ;

}

