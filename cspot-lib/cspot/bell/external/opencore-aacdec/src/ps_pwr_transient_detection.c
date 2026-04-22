

#include    "config.h"

#ifdef AAC_PLUS

#ifdef PARAMETRICSTEREO

#include    "pv_audio_type_defs.h"
#include    "s_ps_dec.h"
#include    "aac_mem_funcs.h"
#include    "ps_all_pass_filter_coeff.h"
#include    "ps_pwr_transient_detection.h"

#include    "fxp_mul32.h"
#include    "pv_div.h"

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#define R_SHIFT     29
#define Q29_fmt(x)   (Int32)(x*((Int32)1<<R_SHIFT) + (x>=0?0.5F:-0.5F))

#define Qfmt31(a)   (Int32)(-a*((Int32)1<<31) - 1 + (a>=0?0.5F:-0.5F))

void ps_pwr_transient_detection(STRUCT_PS_DEC *h_ps_dec,
                                Int32 *rIntBufferLeft,
                                Int32 *iIntBufferLeft,
                                Int32 aTransRatio[])
{

    Int32 sb;
    Int32 maxsb;
    Int32 gr;
    Int32 bin;

    Int32 *aLeftReal;
    Int32 *aLeftImag;
    Int32   temp_r;
    Int32   temp_i;
    Int32   accu;
    Int32 *aPower = aTransRatio;
    Quotient result;

    Int32 nrg;
    Int32 *ptr_aPrevNrg;
    Int32 peakDiff;
    Int32 *ptr_PrevPeakDiff;

    aLeftReal = rIntBufferLeft;
    aLeftImag = iIntBufferLeft;

    for (gr = SUBQMF_GROUPS; gr < NO_IID_GROUPS; gr++)
    {
        maxsb = min(h_ps_dec->usb, groupBorders[ gr+1]);

        accu = 0;

        for (sb = groupBorders[gr]; sb < maxsb; sb++)
        {

            temp_r = aLeftReal[sb];
            temp_i = aLeftImag[sb];
            accu =  fxp_mac32_Q31(accu, temp_r, temp_r);
            accu =  fxp_mac32_Q31(accu, temp_i, temp_i);

        }
        aPower[gr - 2] = accu >> 1;
    }

    aLeftReal = h_ps_dec->mHybridRealLeft;
    aLeftImag = h_ps_dec->mHybridImagLeft;

    temp_r = aLeftReal[0];
    temp_i = aLeftImag[0];
    accu   = fxp_mul32_Q31(temp_r, temp_r);
    accu  = fxp_mac32_Q31(accu, temp_i, temp_i);
    temp_r = aLeftReal[5];
    temp_i = aLeftImag[5];
    accu   = fxp_mac32_Q31(accu, temp_r, temp_r);
    aPower[0]  = fxp_mac32_Q31(accu, temp_i, temp_i) >> 1;

    temp_r = aLeftReal[1];
    temp_i = aLeftImag[1];
    accu   = fxp_mul32_Q31(temp_r, temp_r);
    accu  = fxp_mac32_Q31(accu, temp_i, temp_i);
    temp_r = aLeftReal[4];
    temp_i = aLeftImag[4];
    accu   = fxp_mac32_Q31(accu, temp_r, temp_r);
    aPower[1]  = fxp_mac32_Q31(accu, temp_i, temp_i) >> 1;

    temp_r = aLeftReal[2];
    temp_i = aLeftImag[2];
    accu   = fxp_mul32_Q31(temp_r, temp_r);
    aPower[2]  = fxp_mac32_Q31(accu, temp_i, temp_i) >> 1;

    temp_r = aLeftReal[3];
    temp_i = aLeftImag[3];
    accu   = fxp_mul32_Q31(temp_r, temp_r);
    aPower[3]  = fxp_mac32_Q31(accu, temp_i, temp_i) >> 1;

    temp_r = aLeftReal[6];
    temp_i = aLeftImag[6];
    accu   = fxp_mul32_Q31(temp_r, temp_r);
    aPower[5]  = fxp_mac32_Q31(accu, temp_i, temp_i) >> 1;

    temp_r = aLeftReal[7];
    temp_i = aLeftImag[7];
    accu   = fxp_mul32_Q31(temp_r, temp_r);
    aPower[4]  = fxp_mac32_Q31(accu, temp_i, temp_i) >> 1;

    temp_r = aLeftReal[8];
    temp_i = aLeftImag[8];
    accu   = fxp_mul32_Q31(temp_r, temp_r);
    aPower[6]  = fxp_mac32_Q31(accu, temp_i, temp_i) >> 1;

    temp_r = aLeftReal[9];
    temp_i = aLeftImag[9];
    accu   = fxp_mul32_Q31(temp_r, temp_r);
    aPower[7]  = fxp_mac32_Q31(accu, temp_i, temp_i) >> 1;

    ptr_aPrevNrg = h_ps_dec->aPrevNrg;

    ptr_PrevPeakDiff = h_ps_dec->aPrevPeakDiff;

    for (bin = 0; bin < NO_BINS; bin++)
    {

        peakDiff  = *ptr_PrevPeakDiff;

        accu = h_ps_dec->aPeakDecayFast[bin];
        peakDiff -= peakDiff >> 2;

        accu  = fxp_mul32_Q31(accu, Qfmt31(0.765928338364649f)) << 1;

        if (accu < *aPower)
        {
            accu = *aPower;
        }
        else
        {
            peakDiff += ((accu - *aPower) >> 2);
        }

        h_ps_dec->aPeakDecayFast[bin] = accu;

        *(ptr_PrevPeakDiff++) = peakDiff;

        nrg =   *ptr_aPrevNrg + ((*aPower - *ptr_aPrevNrg) >> 2);

        *(ptr_aPrevNrg++) = nrg;

        peakDiff += peakDiff >> 1;

        if (peakDiff <= nrg)
        {
            *(aPower++) = 0x7FFFFFFF;
        }
        else
        {
            pv_div(nrg, peakDiff, &result);
            *(aPower++) = (result.quotient >> (result.shift_factor)) << 1;
        }

    }

}

#endif

#endif
