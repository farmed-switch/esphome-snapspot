

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_create_limiter_bands.h"
#include    "shellsort.h"
#include    "s_patch.h"
#include    "pv_log2.h"

#include "fxp_mul32.h"

#define R_SHIFT     29
#define Q_fmt(x)   (Int32)(x*((Int32)1<<R_SHIFT) + (x>=0?0.5F:-0.5F))

void sbr_create_limiter_bands(Int32 limSbc[][13],
                              Int32 *gateMode,
                              Int   *freqTable,
                              struct PATCH Patch,
                              const Int32 noBands)
{
    Int32 i;
    Int32 j;
    Int32 k;
    Int isPatchBorder[2];
    Int32 patchBorders[MAX_NUM_PATCHES + 1];
    Int32 workLimiterBandTable[32 + MAX_NUM_PATCHES + 1];

    Int32 nOctaves;
    const Int32 limiterBandsPerOctave[4] =
        {Q_fmt(0.0F), Q_fmt(1.2F),
         Q_fmt(2.0F), Q_fmt(3.0F)
        };

    Int32 tmp_q1;

    Int32 noPatches = Patch.noOfPatches;
    Int32 lowSubband = freqTable[0];
    Int32 highSubband = freqTable[noBands];

    for (i = 0; i < noPatches; i++)
    {
        patchBorders[i] = Patch.targetStartBand[i] - lowSubband;
    }
    patchBorders[i] = highSubband - lowSubband;

    limSbc[0][0] = freqTable[0] - lowSubband;
    limSbc[0][1] = freqTable[noBands] - lowSubband;
    gateMode[0] = 1;

    for (i = 1; i < 4; i++)
    {

        for (k = 0; k <= noBands; k++)
        {
            workLimiterBandTable[k] = freqTable[k] - lowSubband;
        }

        for (k = 1; k < noPatches; k++)
        {
            workLimiterBandTable[noBands+k] = patchBorders[k];
        }

        gateMode[i] = noBands + noPatches - 1;
        shellsort(workLimiterBandTable, gateMode[i] + 1);

        for (j = 1; j <= gateMode[i]; j++)
        {
            tmp_q1 = ((workLimiterBandTable[j] + lowSubband) << 20) / (workLimiterBandTable[j-1] + lowSubband);

            nOctaves = pv_log2(tmp_q1);

            tmp_q1 = fxp_mul32_Q20(nOctaves, limiterBandsPerOctave[i]);
            if (tmp_q1 < Q_fmt(0.49))
            {
                if (workLimiterBandTable[j] == workLimiterBandTable[j-1])
                {
                    workLimiterBandTable[j] = highSubband;
                    shellsort(workLimiterBandTable, gateMode[i] + 1);
                    gateMode[i]--;
                    j--;
                    continue;
                }

                isPatchBorder[0] = isPatchBorder[1] = 0;

                for (k = 0; k <= noPatches; k++)
                {
                    if (workLimiterBandTable[j-1] == patchBorders[k])
                    {
                        isPatchBorder[0] = 1;
                        break;
                    }
                }

                for (k = 0; k <= noPatches; k++)
                {
                    if (workLimiterBandTable[j] == patchBorders[k])
                    {
                        isPatchBorder[1] = 1;
                        break;
                    }
                }

                if (!isPatchBorder[1])
                {
                    workLimiterBandTable[j] = highSubband;
                    shellsort(workLimiterBandTable, gateMode[i] + 1);
                    gateMode[i]--;
                    j--;
                }
                else if (!isPatchBorder[0])
                {
                    workLimiterBandTable[j-1] = highSubband;
                    shellsort(workLimiterBandTable, gateMode[i] + 1);
                    gateMode[i]--;
                    j--;
                }
            }
        }
        for (k = 0; k <= gateMode[i]; k++)
        {
            limSbc[i][k] = workLimiterBandTable[k];
        }
    }
}

#endif

