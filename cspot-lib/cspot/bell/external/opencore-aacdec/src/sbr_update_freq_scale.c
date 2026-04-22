

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_update_freq_scale.h"
#include    "shellsort.h"

#include    "pv_pow2.h"
#include    "pv_log2.h"

#include "fxp_mul32.h"
#define R_SHIFT     30
#define Q_fmt(x)    (Int32)(x*((Int32)1<<R_SHIFT) + (x>=0?0.5F:-0.5F))
#define Q28fmt(x)   (Int32)(x*((Int32)1<<28) + (x>=0?0.5F:-0.5F))

void sbr_update_freq_scale(Int32 * v_k_master,
                           Int32 *h_num_bands,
                           const Int32 lsbM,
                           const Int32 usb,
                           const Int32 freqScale,
                           const Int32 alterScale,
                           const Int32 channelOffset)
{
    Int32 i;
    Int32 numBands = 0;
    Int32 numBands2;
    Int32 tmp_q1;

    if (freqScale > 0)
    {
        Int32 reg;
        Int32 regions;
        Int32 b_p_o;
        Int32 k[3];
        Int32 d[MAX_SECOND_REGION];
        Int32 d2[MAX_SECOND_REGION];
        Int32 w[2] = {Q_fmt(1.0F), Q_fmt(1.0F)};

        k[0] = lsbM;
        k[1] = usb;
        k[2] = usb;

        b_p_o = (freqScale == 1)  ? 12 : 8;
        b_p_o = (freqScale == 2)  ? 10 : b_p_o;

        w[1]  = (alterScale == 0) ? Q_fmt(0.5f) : Q_fmt(0.384615384615386f);

        if (usb > fxp_mul32_Q28(lsbM, Q28fmt(2.2449)))
        {
            regions = 2;
            k[1] = (lsbM << 1);
        }
        else
        {
            regions = 1;
        }

        *h_num_bands = 0;
        for (reg = 0; reg < regions; reg++)
        {
            if (reg == 0)
            {

                tmp_q1 = pv_log2((k[1] << 20) / k[0]);

                tmp_q1 = fxp_mul32_Q15(tmp_q1, b_p_o);
                tmp_q1 = (tmp_q1 + 32) >> 6;

                numBands = tmp_q1 << 1;

                CalcBands(d, k[0], k[1], numBands);
                shellsort(d, numBands);
                cumSum(k[0] - channelOffset,
                       d,
                       numBands,
                       (v_k_master + *h_num_bands));

                *h_num_bands += numBands;
            }
            else
            {
                tmp_q1 = pv_log2((k[reg + 1] << 20) / k[reg]);

                tmp_q1 = fxp_mul32_Q30(tmp_q1, w[reg]);
                tmp_q1 = fxp_mul32_Q15(tmp_q1, b_p_o);
                tmp_q1 = (tmp_q1 + 16) >> 5;

                numBands2 = tmp_q1 << 1;

                CalcBands(d2, k[reg], k[reg+1], numBands2);
                shellsort(d2, numBands2);
                if (d[numBands-1] > d2[0])
                {

                    Int32   change = d[numBands-1] - d2[0];

                    if (change > (d2[numBands2-1] - d2[0]) >> 1)
                    {
                        change = (d2[numBands2-1] - d2[0]) >> 1;
                    }

                    d2[0] += change;
                    d2[numBands2-1] -= change;
                    shellsort(d2, numBands2);

                }
                cumSum(k[reg] - channelOffset,
                       d2,
                       numBands2,
                       v_k_master + *h_num_bands);

                *h_num_bands += numBands2;
            }
        }
    }
    else
    {
        Int32     k2_achived;
        Int32     k2_diff;
        Int32     diff_tot[MAX_OCTAVE + MAX_SECOND_REGION];
        Int32     dk;
        Int32     incr = 0;

        if (alterScale)
        {
            numBands = (usb - lsbM) >> 1;
            dk = 1;
            k2_achived = lsbM + numBands;
        }
        else
        {
            numBands = usb - lsbM;
            if (numBands & 0x1)
            {
                numBands--;
            }
            dk = 2;
            k2_achived = lsbM + (numBands << 1);
        }

        k2_diff = usb - k2_achived;

        for (i = 0; i < numBands; i++)
        {
            diff_tot[i] = dk;
        }

        if (k2_diff < 0)
        {
            incr = 1;
            i = 0;
        }

        if (k2_diff > 0)
        {
            incr = -1;
            i = numBands - 1;
        }

        while (k2_diff != 0)
        {
            diff_tot[i] -=  incr;
            i += incr;
            k2_diff += incr;
        }

        cumSum(lsbM,
               diff_tot,
               numBands,
               v_k_master);

        *h_num_bands = numBands;
    }
}

void CalcBands(Int32 * diff,
               Int32 start,
               Int32 stop,
               Int32 num_bands)
{
    Int32 i;
    Int32 previous;
    Int32 current;
    Int32 tmp_q1;

    previous = start;

    for (i = 1; i <= num_bands; i++)
    {

        tmp_q1 = pv_log2((stop << 20) / start);

        tmp_q1 = fxp_mul32_Q20(tmp_q1, (i << 27) / num_bands);
        tmp_q1 = pv_pow2(tmp_q1);

        tmp_q1 = fxp_mul32_Q20(tmp_q1, start);

        current = (tmp_q1 + 16) >> 5;

        diff[i-1] = current - previous;
        previous  = current;
    }

}

void cumSum(Int32 start_value,
            Int32 * diff,
            Int32 length,
            Int32 * start_adress)
{
    Int32 i;
    Int32 *pt_start_adress   = start_adress;
    Int32 *pt_start_adress_1 = start_adress;
    Int32 *pt_diff           = diff;

    if (length > 0)
    {
        *(pt_start_adress_1++) = start_value;

        for (i = (length >> 1); i != 0; i--)
        {
            *(pt_start_adress_1++) = *(pt_start_adress++) + *(pt_diff++);
            *(pt_start_adress_1++) = *(pt_start_adress++) + *(pt_diff++);
        }

        if (length&1)
        {
            *(pt_start_adress_1) = *(pt_start_adress) + *(pt_diff);
        }
    }

}

#endif
