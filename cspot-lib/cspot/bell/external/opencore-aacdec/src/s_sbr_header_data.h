

#ifndef S_SBR_HEADER_DATA_H
#define S_SBR_HEADER_DATA_H

#include    "pv_audio_type_defs.h"
#include    "e_sbr_header_status.h"
#include    "e_sbr_master_status.h"
#include    "e_sr_mode.h"

typedef struct
{
    SBR_HEADER_STATUS status;
    SBR_MASTER_STATUS masterStatus;

    Int32 crcEnable;
    SR_MODE sampleRateMode;
    Int32 ampResolution;

    Int32 startFreq;
    Int32 stopFreq;
    Int32 xover_band;
    Int32 freqScale;
    Int32 alterScale;
    Int32 noise_bands;

    Int32 noNoiseBands;

    Int32 limiterBands;
    Int32 limiterGains;
    Int32 interpolFreq;
    Int32 smoothingLength;
}
SBR_HEADER_DATA;

typedef SBR_HEADER_DATA *HANDLE_SBR_HEADER_DATA;

#endif

