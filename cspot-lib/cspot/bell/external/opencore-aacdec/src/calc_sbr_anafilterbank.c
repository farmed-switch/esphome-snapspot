

#include "config.h"

#ifdef AAC_PLUS

#include    "calc_sbr_anafilterbank.h"
#include    "qmf_filterbank_coeff.h"
#include    "analysis_sub_band.h"

#include    "aac_mem_funcs.h"
#include    "fxp_mul32.h"

void calc_sbr_anafilterbank_LC(Int32 * Sr,
                               Int16 * X,
                               Int32 scratch_mem[][64],
                               Int32 maxBand)
{

    Int i;
    Int32   *p_Y_1;
    Int32   *p_Y_2;

    Int16 * pt_X_1;
    Int16 * pt_X_2;
    Int32 realAccu1;
    Int32 realAccu2;

    Int32 tmp1;
    Int32 tmp2;

    const Int32 * pt_C;

    p_Y_1 = scratch_mem[0];

    p_Y_2 = p_Y_1 + 63;
    pt_C   = &sbrDecoderFilterbankCoefficients_an_filt_LC[0];

    pt_X_1 = X;

    realAccu1  =  fxp_mul32_by_16(Qfmt27(-0.51075594183097F),   pt_X_1[-192]);

    realAccu1  =  fxp_mac32_by_16(Qfmt27(-0.51075594183097F), -pt_X_1[-128], realAccu1);
    realAccu1  =  fxp_mac32_by_16(Qfmt27(-0.01876919066980F),  pt_X_1[-256], realAccu1);
    *(p_Y_1++) =  fxp_mac32_by_16(Qfmt27(-0.01876919066980F), -pt_X_1[ -64], realAccu1);

    pt_X_1 = &X[-1];
    pt_X_2 = &X[-319];

    for (i = 15; i != 0; i--)
    {
        tmp1 = *(pt_X_1--);
        tmp2 = *(pt_X_2++);

        realAccu1  = fxp_mul32_by_16(*(pt_C), tmp1);
        realAccu2  = fxp_mul32_by_16(*(pt_C++), tmp2);
        tmp1 = pt_X_1[ -63];
        tmp2 = pt_X_2[ +63];
        realAccu1  = fxp_mac32_by_16(*(pt_C), tmp1, realAccu1);
        realAccu2  = fxp_mac32_by_16(*(pt_C++), tmp2, realAccu2);
        tmp1 = pt_X_1[ -127];
        tmp2 = pt_X_2[ +127];
        realAccu1  = fxp_mac32_by_16(*(pt_C), tmp1, realAccu1);
        realAccu2  = fxp_mac32_by_16(*(pt_C++), tmp2, realAccu2);
        tmp1 = pt_X_1[ -191];
        tmp2 = pt_X_2[ +191];
        realAccu1  = fxp_mac32_by_16(*(pt_C), tmp1, realAccu1);
        realAccu2  = fxp_mac32_by_16(*(pt_C++), tmp2, realAccu2);
        tmp1 = pt_X_1[ -255];
        tmp2 = pt_X_2[ +255];
        *(p_Y_1++) = fxp_mac32_by_16(*(pt_C), tmp1, realAccu1);
        *(p_Y_2--) = fxp_mac32_by_16(*(pt_C++), tmp2, realAccu2);

        tmp1 = *(pt_X_1--);
        tmp2 = *(pt_X_2++);
        realAccu1  = fxp_mul32_by_16(*(pt_C), tmp1);
        realAccu2  = fxp_mul32_by_16(*(pt_C++), tmp2);

        tmp1 = pt_X_1[ -63];
        tmp2 = pt_X_2[ +63];
        realAccu1  = fxp_mac32_by_16(*(pt_C), tmp1, realAccu1);
        realAccu2  = fxp_mac32_by_16(*(pt_C++), tmp2, realAccu2);
        tmp1 = pt_X_1[ -127];
        tmp2 = pt_X_2[ +127];
        realAccu1  = fxp_mac32_by_16(*(pt_C), tmp1, realAccu1);
        realAccu2  = fxp_mac32_by_16(*(pt_C++), tmp2, realAccu2);
        tmp1 = pt_X_1[ -191];
        tmp2 = pt_X_2[ +191];
        realAccu1  = fxp_mac32_by_16(*(pt_C), tmp1, realAccu1);
        realAccu2  = fxp_mac32_by_16(*(pt_C++), tmp2, realAccu2);
        tmp1 = pt_X_1[ -255];
        tmp2 = pt_X_2[ +255];
        *(p_Y_1++) = fxp_mac32_by_16(*(pt_C), tmp1, realAccu1);
        *(p_Y_2--) = fxp_mac32_by_16(*(pt_C++), tmp2, realAccu2);

    }

    tmp1 = *(pt_X_1--);
    tmp2 = *(pt_X_2++);
    realAccu1  = fxp_mul32_by_16(*(pt_C), tmp1);
    realAccu2  = fxp_mul32_by_16(*(pt_C++), tmp2);

    tmp1 = pt_X_1[ -63];
    tmp2 = pt_X_2[ +63];
    realAccu1  = fxp_mac32_by_16(*(pt_C), tmp1, realAccu1);
    realAccu2  = fxp_mac32_by_16(*(pt_C++), tmp2, realAccu2);
    tmp1 = pt_X_1[ -127];
    tmp2 = pt_X_2[ +127];
    realAccu1  = fxp_mac32_by_16(*(pt_C), tmp1, realAccu1);
    realAccu2  = fxp_mac32_by_16(*(pt_C++), tmp2, realAccu2);
    tmp1 = pt_X_1[ -191];
    tmp2 = pt_X_2[ +191];
    realAccu1  = fxp_mac32_by_16(*(pt_C), tmp1, realAccu1);
    realAccu2  = fxp_mac32_by_16(*(pt_C++), tmp2, realAccu2);
    tmp1 = pt_X_1[ -255];
    tmp2 = pt_X_2[ +255];
    *(p_Y_1++) = fxp_mac32_by_16(*(pt_C), tmp1, realAccu1);
    *(p_Y_2--) = fxp_mac32_by_16(*(pt_C++), tmp2, realAccu2);

    pt_X_1 = X;

    realAccu2  = fxp_mul32_by_16(Qfmt27(0.00370548843500F), X[ -32]);

    realAccu2  = fxp_mac32_by_16(Qfmt27(0.00370548843500F), pt_X_1[-288], realAccu2);
    realAccu2  = fxp_mac32_by_16(Qfmt27(0.09949460091720F), pt_X_1[ -96], realAccu2);
    realAccu2  = fxp_mac32_by_16(Qfmt27(0.09949460091720F), pt_X_1[-224], realAccu2);
    *(p_Y_1++) = fxp_mac32_by_16(Qfmt27(1.20736865027288F), pt_X_1[-160], realAccu2);

    analysis_sub_band_LC(scratch_mem[0],
                         Sr,
                         maxBand,
                         (Int32(*)[64])scratch_mem[1]);

}

#ifdef HQ_SBR

void calc_sbr_anafilterbank(Int32 * Sr,
                            Int32 * Si,
                            Int16 * X,
                            Int32 scratch_mem[][64],
                            Int32   maxBand)
{
    Int i;
    Int32   *p_Y_1;
    Int32   *p_Y_2;

    const Int32 * pt_C;
    Int16 * pt_X_1;
    Int16 * pt_X_2;
    Int32 realAccu1;
    Int32 realAccu2;

    Int32 tmp1;
    Int32 tmp2;

    p_Y_1 = scratch_mem[0];

    p_Y_2 = p_Y_1 + 63;
    pt_C   = &sbrDecoderFilterbankCoefficients_an_filt[0];

    realAccu1  =  fxp_mul32_by_16(Qfmt27(-0.36115899F),   X[-192]);

    realAccu1  =  fxp_mac32_by_16(Qfmt27(-0.36115899F),  -X[-128], realAccu1);
    realAccu1  =  fxp_mac32_by_16(Qfmt27(-0.013271822F),  X[-256], realAccu1);
    *(p_Y_1++) =  fxp_mac32_by_16(Qfmt27(-0.013271822F), -X[ -64], realAccu1);

    pt_X_1 = &X[-1];
    pt_X_2 = &X[-319];

    for (i = 31; i != 0; i--)
    {
        tmp1 = *(pt_X_1--);
        tmp2 = *(pt_X_2++);
        realAccu1  = fxp_mul32_by_16(*(pt_C), tmp1);
        realAccu2  = fxp_mul32_by_16(*(pt_C++), tmp2);
        tmp1 = pt_X_1[ -63];
        tmp2 = pt_X_2[  63];
        realAccu1  = fxp_mac32_by_16(*(pt_C), tmp1, realAccu1);
        realAccu2  = fxp_mac32_by_16(*(pt_C++), tmp2, realAccu2);
        tmp1 = pt_X_1[ -127];
        tmp2 = pt_X_2[  127];
        realAccu1  = fxp_mac32_by_16(*(pt_C), tmp1, realAccu1);
        realAccu2  = fxp_mac32_by_16(*(pt_C++), tmp2, realAccu2);
        tmp1 = pt_X_1[ -191];
        tmp2 = pt_X_2[  191];
        realAccu1  = fxp_mac32_by_16(*(pt_C), tmp1, realAccu1);
        realAccu2  = fxp_mac32_by_16(*(pt_C++), tmp2, realAccu2);
        tmp1 = pt_X_1[ -255];
        tmp2 = pt_X_2[  255];
        *(p_Y_1++) = fxp_mac32_by_16(*(pt_C), tmp1, realAccu1);
        *(p_Y_2--) = fxp_mac32_by_16(*(pt_C++), tmp2, realAccu2);
    }

    realAccu2  = fxp_mul32_by_16(Qfmt27(0.002620176F), X[ -32]);
    realAccu2  = fxp_mac32_by_16(Qfmt27(0.002620176F), X[-288], realAccu2);
    realAccu2  = fxp_mac32_by_16(Qfmt27(0.070353307F), X[ -96], realAccu2);
    realAccu2  = fxp_mac32_by_16(Qfmt27(0.070353307F), X[-224], realAccu2);

    *(p_Y_1++) = fxp_mac32_by_16(Qfmt27(0.85373856F), (X[-160]), realAccu2);

    analysis_sub_band(scratch_mem[0],
                      Sr,
                      Si,
                      maxBand,
                      (Int32(*)[64])scratch_mem[1]);

}

#endif

#endif

