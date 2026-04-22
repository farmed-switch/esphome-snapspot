

#include "pv_audio_type_defs.h"
#include "ibstream.h"
#include "s_pulseinfo.h"
#include "s_bits.h"
#include "e_rawbitstreamconst.h"
#include "get_pulse_data.h"

Int get_pulse_data(
    PulseInfo   *pPulseInfo,
    BITS        *pInputStream)
{
    Int   i;
    Int  *pPulseOffset;
    Int  *pPulseAmp;
    Int   status = SUCCESS;
    UInt  temp;

    temp =
        get9_n_lessbits(
            LEN_PULSE_NPULSE + LEN_PULSE_ST_SFB,
            pInputStream);

    pPulseInfo->number_pulse = (Int)(1 + (temp >> LEN_PULSE_ST_SFB));
    pPulseInfo->pulse_start_sfb = (Int)(temp & ((1 << LEN_PULSE_ST_SFB) - 1));

    pPulseOffset = &pPulseInfo->pulse_offset[0];
    pPulseAmp    = &pPulseInfo->pulse_amp[0];

    for (i = pPulseInfo->number_pulse; i > 0; i--)
    {

        temp =
            get9_n_lessbits(
                LEN_PULSE_POFF + LEN_PULSE_PAMP,
                pInputStream);

        *pPulseOffset++ = (Int)(temp >> LEN_PULSE_PAMP);

        *pPulseAmp++    = (Int)(temp & ((1 << LEN_PULSE_PAMP) - 1));
    }

    return (status);
}

