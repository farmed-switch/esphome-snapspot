

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_inv_filt_levelemphasis.h"
#include    "sbr_generate_high_freq.h"

#include "pv_audio_type_defs.h"
#include "fxp_mul32.h"

#define R_SHIFT     29
#define Qfmt(x)   (Int32)(x*((Int32)1<<R_SHIFT) + (x>=0?0.5F:-0.5F))

const Int32 InvFiltFactors[5] = {Qfmt(0.00f),
                                 Qfmt(0.60f),
                                 Qfmt(0.75f),
                                 Qfmt(0.90f),
                                 Qfmt(0.98f)
                                };

void sbr_inv_filt_levelemphasis(INVF_MODE *invFiltMode,
                                INVF_MODE *prevInvFiltMode,
                                Int32 nNfb,
                                Int32  BwVector[MAX_NUM_PATCHES],
                                Int32  BwVectorOld[MAX_NUM_PATCHES])
{
    Int32 i;
    Int32 j;
    Int32 tmp;

    for (i = 0; i < nNfb; i++)
    {
        switch (invFiltMode[i])
        {
            case INVF_LOW_LEVEL:
                if (prevInvFiltMode[i] == INVF_OFF)
                {
                    j = 1;
                }
                else
                {
                    j = 2;
                }
                break;

            case INVF_MID_LEVEL:
                j = 3;
                break;

            case INVF_HIGH_LEVEL:
                j = 4;
                break;

            default:
                if (prevInvFiltMode[i] == INVF_LOW_LEVEL)
                {
                    j = 1;
                }
                else
                {
                    j = 0;
                }
        }

        tmp  =  InvFiltFactors[j];

        if (tmp < BwVectorOld[i])
        {
            tmp = ((tmp << 1) + tmp + BwVectorOld[i]) >> 2;
        }
        else
        {
            tmp =  fxp_mul32_Q29(Qfmt(0.90625f), tmp);
            tmp =  fxp_mac32_Q29(Qfmt(0.09375f), BwVectorOld[i], tmp);
        }

        if (tmp < Qfmt(0.015625F))
        {
            tmp = 0;
        }

        if (tmp >= Qfmt(0.99609375f))
        {
            tmp = Qfmt(0.99609375f);
        }

        BwVector[i] = tmp;
    }
}

#endif

