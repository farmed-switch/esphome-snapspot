

#include "config.h"

#ifdef AAC_PLUS

#include    "decode_noise_floorlevels.h"
#include    "sbr_constants.h"

void decode_noise_floorlevels(SBR_FRAME_DATA * hFrameData)

{
    Int32 env;
    Int32 i;

    Int32 * frameInfo           = hFrameData->frameInfo;
    Int32   nNfb                = hFrameData->nNfb;
    Int32 * domain_vec          = hFrameData->domain_vec2;

    Int32 * sbrNoiseFloorLevel_man = hFrameData->sbrNoiseFloorLevel_man;
    Int32 * prevNoiseLevel_man     = hFrameData->prevNoiseLevel_man;

    Int32 nEnv = frameInfo[(frameInfo[0] << 1) + 3];

    for (env = 0; env < nEnv; env++)
    {
        if (domain_vec[env] == 0)
        {
            prevNoiseLevel_man[0] = *(sbrNoiseFloorLevel_man++);

            for (i = 1; i < nNfb; i++)
            {
                *sbrNoiseFloorLevel_man += *(sbrNoiseFloorLevel_man - 1);
                prevNoiseLevel_man[i] = *(sbrNoiseFloorLevel_man++);
            }
        }
        else
        {
            for (i = 0; i < nNfb; i++)
            {
                *sbrNoiseFloorLevel_man += prevNoiseLevel_man[i];
                prevNoiseLevel_man[i] = *(sbrNoiseFloorLevel_man++);
            }
        }

    }
}

#endif
