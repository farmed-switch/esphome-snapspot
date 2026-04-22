

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_envelope_unmapping.h"
#include    "sbr_constants.h"

#include "fxp_mul32.h"

#define R_SHIFT     30
#define Qfmt(x)   (Int32)(x*((Int32)1<<R_SHIFT) + (x>=0?0.5F:-0.5F))

const Int32 one_over_one_plus_two_to_n[11] =
{
    Qfmt(0.50000000000000F), Qfmt(0.66666666666667F), Qfmt(0.80000000000000F),
    Qfmt(0.88888888888889F), Qfmt(0.94117647058824F), Qfmt(0.96969696969697F),
    Qfmt(0.98461538461538F), Qfmt(0.99224806201550F), Qfmt(0.99610894941634F),
    Qfmt(0.99805068226121F), Qfmt(0.99902439024390F)
};

const Int32 one_over_one_plus_sq_2_by_two_to_n[12] =
{
    Qfmt(0.41421356237310F), Qfmt(0.58578643762690F), Qfmt(0.73879612503626F),
    Qfmt(0.84977889517767F), Qfmt(0.91878969685839F), Qfmt(0.95767628767521F),
    Qfmt(0.97838063800882F), Qfmt(0.98907219289563F), Qfmt(0.99450607818892F),
    Qfmt(0.99724547251514F), Qfmt(0.99862083678608F), Qfmt(0.99930994254211F)
};

void sbr_envelope_unmapping(SBR_FRAME_DATA * hFrameData1,
                            SBR_FRAME_DATA * hFrameData2)

{
    Int32 i;
    Int32 tempLeft;
    Int32 tempRight;

    Int32 tmp;
    Int32 *iEnvelopeLeft_man    = hFrameData1->iEnvelope_man;
    Int32 *iEnvelopeLeft_exp    = hFrameData1->iEnvelope_exp;
    Int32 *noiseFloorLeft_man   = hFrameData1->sbrNoiseFloorLevel_man;
    Int32 *noiseFloorLeft_exp   = hFrameData1->sbrNoiseFloorLevel_exp;

    Int32 *iEnvelopeRight_man   = hFrameData2->iEnvelope_man;
    Int32 *iEnvelopeRight_exp   = hFrameData2->iEnvelope_exp;
    Int32 *noiseFloorRight_man  = hFrameData2->sbrNoiseFloorLevel_man;
    Int32 *noiseFloorRight_exp  = hFrameData2->sbrNoiseFloorLevel_exp;

    if (hFrameData2->ampRes)
    {
        for (i = 0; i < hFrameData1->nScaleFactors; i++)
        {
            tempRight = iEnvelopeRight_man[i];
            tempLeft  = iEnvelopeLeft_man[i];

            iEnvelopeLeft_exp[i] = tempLeft + 7;

            iEnvelopeRight_exp[i] = tempRight - 12;
            iEnvelopeRight_man[i] = Qfmt(1.000F);

            if (iEnvelopeRight_exp[i] >= 0)
            {
                if (iEnvelopeRight_exp[i] < 11)
                {
                    iEnvelopeRight_man[i] = one_over_one_plus_two_to_n[ iEnvelopeRight_exp[i]];
                }
                else
                {
                    iEnvelopeRight_man[i] -= (Qfmt(1.000F) >> iEnvelopeRight_exp[i]);
                }
                iEnvelopeRight_exp[i] = iEnvelopeLeft_exp[i] - iEnvelopeRight_exp[i];
            }
            else
            {
                if (iEnvelopeRight_exp[i] > -11)
                {
                    iEnvelopeRight_man[i] -= one_over_one_plus_two_to_n[ -iEnvelopeRight_exp[i]];
                    iEnvelopeRight_exp[i] = iEnvelopeLeft_exp[i] - iEnvelopeRight_exp[i];

                }
                else
                {
                    iEnvelopeRight_exp[i] = iEnvelopeLeft_exp[i];
                    iEnvelopeLeft_exp[i] = 0;
                }
            }

            iEnvelopeLeft_man[i]  = iEnvelopeRight_man[i];
        }
    }
    else
    {
        for (i = 0; i < hFrameData1->nScaleFactors; i++)
        {

            tempRight = iEnvelopeRight_man[i];
            tempLeft  = iEnvelopeLeft_man[i];

            iEnvelopeLeft_exp[i] = (tempLeft >> 1) + 7;
            if (tempLeft & 0x1)
            {
                iEnvelopeLeft_man[i] = Qfmt(1.41421356237310F);
            }
            else
            {
                iEnvelopeLeft_man[i] = Qfmt(1.000F);
            }

            iEnvelopeRight_exp[i] = (tempRight >> 1) - 12;
            if (tempRight & 0x1)
            {
                if (iEnvelopeRight_exp[i] > 0)
                {
                    iEnvelopeRight_man[i] = Qfmt(1.41421356237310F);
                }
                else
                {
                    iEnvelopeRight_man[i] = Qfmt(0.7071067811865F);
                }
            }
            else
            {
                iEnvelopeRight_man[i] = Qfmt(1.000F);
            }

            if (iEnvelopeRight_man[i] == Qfmt(1.000F))
            {

                if (iEnvelopeRight_exp[i] >= 0)
                {
                    if (iEnvelopeRight_exp[i] < 11)
                    {
                        iEnvelopeRight_man[i] = one_over_one_plus_two_to_n[ iEnvelopeRight_exp[i]];
                    }
                    else
                    {
                        iEnvelopeRight_man[i] -= (Qfmt(1.000F) >> iEnvelopeRight_exp[i]);
                    }
                    iEnvelopeRight_exp[i] = iEnvelopeLeft_exp[i] - iEnvelopeRight_exp[i];

                }
                else
                {
                    if (iEnvelopeRight_exp[i] > -11)
                    {
                        iEnvelopeRight_man[i] -= one_over_one_plus_two_to_n[ -iEnvelopeRight_exp[i]];
                        iEnvelopeRight_exp[i] = iEnvelopeLeft_exp[i] - iEnvelopeRight_exp[i];
                    }
                    else
                    {
                        iEnvelopeRight_exp[i] = iEnvelopeLeft_exp[i];
                        iEnvelopeLeft_exp[i]  = 0;
                    }
                }

                if (iEnvelopeLeft_man[i] != Qfmt(1.000F))
                {
                    iEnvelopeRight_man[i] = fxp_mul32_Q30(iEnvelopeLeft_man[i], iEnvelopeRight_man[i]);
                }

                iEnvelopeLeft_man[i]  = iEnvelopeRight_man[i];

            }
            else
            {

                if (iEnvelopeRight_exp[i] >= 0)
                {
                    if (iEnvelopeRight_exp[i] < 12)
                    {
                        iEnvelopeRight_man[i] = one_over_one_plus_sq_2_by_two_to_n[ iEnvelopeRight_exp[i]];
                    }
                    else
                    {
                        iEnvelopeRight_man[i] = Qfmt(1.000F) - (Qfmt(1.000F) >> iEnvelopeRight_exp[i]);
                    }
                }
                else
                {
                    if (iEnvelopeRight_exp[i] > -12)
                    {
                        iEnvelopeRight_man[i] = Qfmt(1.000F) - one_over_one_plus_sq_2_by_two_to_n[ -iEnvelopeRight_exp[i]];
                    }
                    else
                    {
                        iEnvelopeRight_man[i] = Qfmt(1.000F);
                        iEnvelopeRight_exp[i] = 0;
                    }
                }

                iEnvelopeRight_exp[i] = iEnvelopeLeft_exp[i] - iEnvelopeRight_exp[i];

                if (iEnvelopeLeft_man[i] != Qfmt(1.000F))
                {

                    tmp = iEnvelopeRight_man[i];
                    iEnvelopeRight_man[i] = fxp_mul32_Q30(iEnvelopeLeft_man[i], iEnvelopeRight_man[i]);
                    iEnvelopeLeft_man[i] = tmp;
                    iEnvelopeLeft_exp[i] += 1;
                }
                else
                {
                    iEnvelopeLeft_man[i]  = fxp_mul32_Q30(iEnvelopeRight_man[i], Qfmt(1.41421356237310F));
                }

            }
        }
    }

    for (i = 0; i < hFrameData1->nNoiseFactors; i++)
    {

        noiseFloorLeft_exp[i]  = NOISE_FLOOR_OFFSET_PLUS_1 - noiseFloorLeft_man[i];
        noiseFloorRight_exp[i] = noiseFloorRight_man[i] - SBR_ENERGY_PAN_OFFSET_INT;

        if (noiseFloorRight_exp[i] >= 0)
        {
            if (noiseFloorRight_exp[i] < 11)
            {
                noiseFloorRight_man[i] = one_over_one_plus_two_to_n[ noiseFloorRight_exp[i]];
            }
            else
            {
                noiseFloorRight_man[i] = Qfmt(1.000F) - (Qfmt(1.000F) >> noiseFloorRight_exp[i]);
            }
        }
        else
        {
            if (noiseFloorRight_exp[i] > -11)
            {
                noiseFloorRight_man[i] = Qfmt(1.000F) - one_over_one_plus_two_to_n[ -noiseFloorRight_exp[i]];
            }
            else
            {
                noiseFloorRight_man[i] = Qfmt(1.000F);
                noiseFloorRight_exp[i] = 0;
            }
        }

        noiseFloorRight_exp[i] = noiseFloorLeft_exp[i] - noiseFloorRight_exp[i];

        noiseFloorLeft_man[i] = noiseFloorRight_man[i];
        noiseFloorLeft_exp[i] = noiseFloorLeft_exp[i];

    }
}

#endif

