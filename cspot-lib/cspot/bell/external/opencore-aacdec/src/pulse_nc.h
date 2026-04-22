

#ifndef PULSE_NC_H
#define PULSE_NC_H

#include "pv_audio_type_defs.h"
#include "s_frameinfo.h"
#include "s_pulseinfo.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void pulse_nc(
        Int16        coef[],
        const PulseInfo  *pPulseInfo,
        const FrameInfo  *pLongFrameInfo,
        Int      *max);

#ifdef __cplusplus
}
#endif

#endif

