

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_constants.h"
#include    "sbr_requantize_envelope_data.h"

#define R_SHIFT     30
#define Qfmt(x)   (Int32)(x*((Int32)1<<R_SHIFT) + (x>=0?0.5F:-0.5F))

void sbr_requantize_envelope_data(SBR_FRAME_DATA * hFrameData)

{
    Int32 i;

    Int32  nScaleFactors      =  hFrameData->nScaleFactors;
    Int32  nNoiseFactors      =  hFrameData->nNoiseFactors;
    Int32  ampRes             =  hFrameData->ampRes;
    Int32 *iEnvelope_man      =  hFrameData->iEnvelope_man;
    Int32 *iEnvelope_exp      =  hFrameData->iEnvelope_exp;
    Int32 *sbrNoiseFloorLevel_man = hFrameData->sbrNoiseFloorLevel_man;
    Int32 *sbrNoiseFloorLevel_exp = hFrameData->sbrNoiseFloorLevel_exp;

    if (ampRes)
    {

        for (i = 0; i < nScaleFactors; i++)
        {

            iEnvelope_exp[i] = iEnvelope_man[i] + 6;
            iEnvelope_man[i] = Qfmt(1.000F);
        }
    }
    else
    {

        for (i = 0; i < nScaleFactors; i++)
        {
            iEnvelope_exp[i] = (iEnvelope_man[i] >> 1) + 6;
            if (iEnvelope_man[i] & 0x1)
            {
                iEnvelope_man[i] = Qfmt(1.41421356237310F);
            }
            else
            {
                iEnvelope_man[i] = Qfmt(1.000F);
            }
        }

    }
    for (i = 0; i < nNoiseFactors; i++)
    {

        sbrNoiseFloorLevel_exp[i] = NOISE_FLOOR_OFFSET - sbrNoiseFloorLevel_man[i];
        sbrNoiseFloorLevel_man[i] = 0x40000000;
    }
}

#endif

