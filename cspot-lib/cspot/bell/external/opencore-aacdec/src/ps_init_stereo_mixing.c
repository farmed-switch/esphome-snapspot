

#include    "config.h"

#ifdef AAC_PLUS

#ifdef PARAMETRICSTEREO

#include    "pv_audio_type_defs.h"
#include    "fxp_mul32.h"

#include    "aac_mem_funcs.h"
#include    "pv_sine.h"
#include    "s_ps_dec.h"
#include    "ps_all_pass_filter_coeff.h"
#include    "ps_init_stereo_mixing.h"

#define R_SHIFT     30
#define Q30_fmt(x)   (Int32)(x*((Int32)1<<R_SHIFT) + (x>=0?0.5F:-0.5F))

const Int32 scaleFactors[NO_IID_LEVELS] =
{
    Q30_fmt(1.411983f),  Q30_fmt(1.403138f),  Q30_fmt(1.386877f),
    Q30_fmt(1.348400f),  Q30_fmt(1.291249f),  Q30_fmt(1.196037f),
    Q30_fmt(1.107372f),  Q30_fmt(1.000000f),  Q30_fmt(0.879617f),
    Q30_fmt(0.754649f),  Q30_fmt(0.576780f),  Q30_fmt(0.426401f),
    Q30_fmt(0.276718f),  Q30_fmt(0.176645f),  Q30_fmt(0.079402f)
};

const Int32 scaleFactorsFine[NO_IID_LEVELS_FINE] =
{
    Q30_fmt(1.414207f),  Q30_fmt(1.414191f),  Q30_fmt(1.414143f),
    Q30_fmt(1.413990f),  Q30_fmt(1.413507f),  Q30_fmt(1.411983f),
    Q30_fmt(1.409773f),  Q30_fmt(1.405395f),  Q30_fmt(1.396780f),
    Q30_fmt(1.380053f),  Q30_fmt(1.348400f),  Q30_fmt(1.313920f),
    Q30_fmt(1.264310f),  Q30_fmt(1.196037f),  Q30_fmt(1.107372f),
    Q30_fmt(1.000000f),  Q30_fmt(0.879617f),  Q30_fmt(0.754649f),
    Q30_fmt(0.633656f),  Q30_fmt(0.523081f),  Q30_fmt(0.426401f),
    Q30_fmt(0.308955f),  Q30_fmt(0.221375f),  Q30_fmt(0.157688f),
    Q30_fmt(0.111982f),  Q30_fmt(0.079402f),  Q30_fmt(0.044699f),
    Q30_fmt(0.025145f),  Q30_fmt(0.014141f),  Q30_fmt(0.007953f),
    Q30_fmt(0.004472f)
};

const Int32 scaled_alphas[NO_ICC_LEVELS] =
{
    Q30_fmt(0.00000000000000f),  Q30_fmt(0.12616764875355f),
    Q30_fmt(0.20199707286122f),  Q30_fmt(0.32744135137762f),
    Q30_fmt(0.42225800677370f),  Q30_fmt(0.55536025173035f),
    Q30_fmt(0.77803595530059f),  Q30_fmt(1.11072050346071f)
};

const Int32 cos_alphas[NO_ICC_LEVELS] =
{
    Q30_fmt(1.00000000000000f),  Q30_fmt(0.98412391153249f),
    Q30_fmt(0.95947390717984f),  Q30_fmt(0.89468446298319f),
    Q30_fmt(0.82693418207478f),  Q30_fmt(0.70710689672598f),
    Q30_fmt(0.45332071670080f),  Q30_fmt(0.00000032679490f)
};

const Int32 sin_alphas[NO_ICC_LEVELS] =
{
    Q30_fmt(0.00000000000000f),  Q30_fmt(0.17748275057029f),
    Q30_fmt(0.28179748302823f),  Q30_fmt(0.44669868110000f),
    Q30_fmt(0.56229872711603f),  Q30_fmt(0.70710666564709f),
    Q30_fmt(0.89134747871404f),  Q30_fmt(1.00000000000000f)
};

Int32 ps_init_stereo_mixing(STRUCT_PS_DEC *pms,
                            Int32 env,
                            Int32 usb)
{
    Int32   group;
    Int32   bin;
    Int32   noIidSteps;
    Int32   tmp;

    Int32   invEnvLength;
    const Int32  *pScaleFactors;
    Int32   scaleR;
    Int32   scaleL;
    Int32   cos_alpha;
    Int32   sin_alpha;
    Int32   beta;
    Int32   cos_beta;
    Int32   sin_beta;
    Int32   temp1;
    Int32   temp2;
    Int32   *ptr_tmp;
    Int32   h11;
    Int32   h12;
    Int32   h21;
    Int32   h22;

    if (pms->bFineIidQ)
    {
        noIidSteps = NO_IID_STEPS_FINE;
        pScaleFactors = scaleFactorsFine;
    }
    else
    {
        noIidSteps = NO_IID_STEPS;
        pScaleFactors = scaleFactors;
    }

    if (env == 0)
    {
        pms->lastUsb = pms->usb;
        pms->usb = usb;
        if (usb != pms->lastUsb && pms->lastUsb != 0)
        {
            return(-1);

        }
    }

    invEnvLength =  pms->aEnvStartStop[env + 1] - pms->aEnvStartStop[env];

    if (invEnvLength == (Int32) pms->noSubSamples)
    {
        invEnvLength = pms->invNoSubSamples;
    }
    else
    {
        invEnvLength = Q30_fmt(1.0f) / invEnvLength;
    }

    if (invEnvLength == 32)
    {
        for (group = 0; group < NO_IID_GROUPS; group++)
        {
            bin = bins2groupMap[group];

            tmp = pms->aaIidIndex[env][bin];

            scaleR = pScaleFactors[noIidSteps + tmp];

            scaleL = pScaleFactors[noIidSteps - tmp];

            tmp = pms->aaIccIndex[env][bin];

            cos_alpha = cos_alphas[ tmp];
            sin_alpha = sin_alphas[ tmp];

            beta   = fxp_mul32_Q30(scaled_alphas[ tmp], (scaleR - scaleL));

            cos_beta = pv_cosine(beta);
            sin_beta = pv_sine(beta);

            temp1 = fxp_mul32_Q30(cos_beta, cos_alpha);
            temp2 = fxp_mul32_Q30(sin_beta, sin_alpha);

            h11 = fxp_mul32_Q30(scaleL, (temp1 - temp2));
            h12 = fxp_mul32_Q30(scaleR, (temp1 + temp2));

            temp1 = fxp_mul32_Q30(sin_beta, cos_alpha);
            temp2 = fxp_mul32_Q30(cos_beta, sin_alpha);

            h21 = fxp_mul32_Q30(scaleL, (temp1 + temp2));
            h22 = fxp_mul32_Q30(scaleR, (temp1 - temp2));

            ptr_tmp = &pms->h11Prev[group];
            pms->H11[group]       = *ptr_tmp;
            pms->deltaH11[group]  = (h11 - *ptr_tmp) >> 5;
            *ptr_tmp              = h11;

            ptr_tmp = &pms->h12Prev[group];
            pms->H12[group]       = *ptr_tmp;
            pms->deltaH12[group]  = (h12 - *ptr_tmp) >> 5;
            *ptr_tmp              = h12;

            ptr_tmp = &pms->h21Prev[group];
            pms->H21[group]       = *ptr_tmp;
            pms->deltaH21[group]  = (h21 - *ptr_tmp) >> 5;
            *ptr_tmp              = h21;

            ptr_tmp = &pms->h22Prev[group];
            pms->H22[group]       = *ptr_tmp;
            pms->deltaH22[group]  = (h22 - *ptr_tmp) >> 5;
            *ptr_tmp              = h22;

        }
    }
    else
    {

        for (group = 0; group < NO_IID_GROUPS; group++)
        {
            bin = bins2groupMap[group];

            tmp = pms->aaIidIndex[env][bin];

            scaleR = pScaleFactors[noIidSteps + tmp];

            scaleL = pScaleFactors[noIidSteps - tmp];

            tmp = pms->aaIccIndex[env][bin];

            cos_alpha = cos_alphas[ tmp];
            sin_alpha = sin_alphas[ tmp];

            beta   = fxp_mul32_Q30(scaled_alphas[ tmp], (scaleR - scaleL));

            cos_beta = pv_cosine(beta);
            sin_beta = pv_sine(beta);

            temp1 = fxp_mul32_Q30(cos_beta, cos_alpha);
            temp2 = fxp_mul32_Q30(sin_beta, sin_alpha);

            h11 = fxp_mul32_Q30(scaleL, (temp1 - temp2));
            h12 = fxp_mul32_Q30(scaleR, (temp1 + temp2));

            temp1 = fxp_mul32_Q30(sin_beta, cos_alpha);
            temp2 = fxp_mul32_Q30(cos_beta, sin_alpha);

            h21 = fxp_mul32_Q30(scaleL, (temp1 + temp2));
            h22 = fxp_mul32_Q30(scaleR, (temp1 - temp2));

            ptr_tmp = &pms->h11Prev[group];
            pms->deltaH11[group]  = fxp_mul32_Q30((h11 - *ptr_tmp), invEnvLength);
            pms->H11[group]       = *ptr_tmp;
            *ptr_tmp              = h11;

            ptr_tmp = &pms->h12Prev[group];
            pms->deltaH12[group]  = fxp_mul32_Q30((h12 - *ptr_tmp), invEnvLength);
            pms->H12[group]       = *ptr_tmp;
            *ptr_tmp              = h12;

            ptr_tmp = &pms->h21Prev[group];
            pms->deltaH21[group]  = fxp_mul32_Q30((h21 - *ptr_tmp), invEnvLength);
            pms->H21[group]       = *ptr_tmp;
            *ptr_tmp              = h21;

            ptr_tmp = &pms->h22Prev[group];
            pms->deltaH22[group]  = fxp_mul32_Q30((h22 - *ptr_tmp), invEnvLength);
            pms->H22[group]       = *ptr_tmp;
            *ptr_tmp              = h22;

        }
    }

    return (0);

}

#endif

#endif

