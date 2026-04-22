

#ifndef GETACTUALAACCONFIG_H
#define GETACTUALAACCONFIG_H

#include "pv_audio_type_defs.h"

OSCL_IMPORT_REF Int32 GetActualAacConfig(UInt8 *aConfigHeader,
        UInt8 *aAudioObjectType,
        Int32 *aConfigHeaderSize,
        UInt8 *SamplingRateIndex,
        UInt32 *NumChannels);

#endif
