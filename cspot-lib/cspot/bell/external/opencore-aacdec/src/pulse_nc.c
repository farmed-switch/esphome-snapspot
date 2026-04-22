

#include "pv_audio_type_defs.h"
#include "s_frameinfo.h"
#include "s_pulseinfo.h"
#include "pulse_nc.h"

void pulse_nc(
    Int16      coef[],
    const PulseInfo  *pPulseInfo,
    const FrameInfo  *pLongFrameInfo,
    Int      *max)
{
    Int index;

    Int16 *pCoef;
    Int temp;

    const Int *pPulseOffset;
    const Int *pPulseAmp;

    if (pPulseInfo->pulse_start_sfb > 0)
    {
        index = pLongFrameInfo->win_sfb_top[0][pPulseInfo->pulse_start_sfb - 1];
    }
    else
    {
        index = 0;
    }

    pCoef        = &(coef[index]);

    pPulseOffset = &(pPulseInfo->pulse_offset[0]);

    pPulseAmp    = &(pPulseInfo->pulse_amp[0]);

    for (index = pPulseInfo->number_pulse; index > 0; index--)
    {
        pCoef  += *pPulseOffset++;

        temp = *pCoef;

        if (temp > 0)
        {
            temp += *(pPulseAmp++);
            *pCoef = (Int16)temp;
            if (temp > *max)
            {
                *max = temp;
            }
        }
        else
        {
            temp -= *(pPulseAmp++);
            *pCoef = (Int16)temp;
            if (-temp > *max)
            {
                *max = -temp;
            }
        }

    }

    return;

}
