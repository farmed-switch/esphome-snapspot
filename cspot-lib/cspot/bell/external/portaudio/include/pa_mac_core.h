#ifndef PA_MAC_CORE_H
#define PA_MAC_CORE_H

#include "portaudio.h"

#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    unsigned long size;
    PaHostApiTypeId hostApiType;
    unsigned long version;
    unsigned long flags;
    SInt32 const * channelMap;
    unsigned long channelMapSize;
} PaMacCoreStreamInfo;

void PaMacCore_SetupStreamInfo( PaMacCoreStreamInfo *data, unsigned long flags );

void PaMacCore_SetupChannelMap( PaMacCoreStreamInfo *data, const SInt32 * const channelMap, unsigned long channelMapSize );

AudioDeviceID PaMacCore_GetStreamInputDevice( PaStream* s );

AudioDeviceID PaMacCore_GetStreamOutputDevice( PaStream* s );

const char *PaMacCore_GetChannelName( int device, int channelIndex, bool input );

PaError PaMacCore_GetBufferSizeRange( PaDeviceIndex device,
                                       long *minBufferSizeFrames, long *maxBufferSizeFrames );

#define paMacCoreChangeDeviceParameters (0x01)

#define paMacCoreFailIfConversionRequired (0x02)

#define paMacCoreConversionQualityMin    (0x0100)
#define paMacCoreConversionQualityMedium (0x0200)
#define paMacCoreConversionQualityLow    (0x0300)
#define paMacCoreConversionQualityHigh   (0x0400)
#define paMacCoreConversionQualityMax    (0x0000)

#define paMacCorePlayNice                    (0x00)

#define paMacCorePro                         (0x01)

#define paMacCoreMinimizeCPUButPlayNice      (0x0100)

#define paMacCoreMinimizeCPU                 (0x0101)

#ifdef __cplusplus
}
#endif

#endif
