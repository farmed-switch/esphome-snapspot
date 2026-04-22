

#ifndef GET_PULSE_DATA_H
#define GET_PULSE_DATA_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "pv_audio_type_defs.h"
#include "s_pulseinfo.h"
#include "s_bits.h"

    Int get_pulse_data(
        PulseInfo   *pPulseInfo,
        BITS        *pInputStream);

#ifdef __cplusplus
}
#endif

#endif

