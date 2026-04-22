#ifndef PA_WIN_WMME_H
#define PA_WIN_WMME_H

#include "portaudio.h"
#include "pa_win_waveformat.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define paWinMmeUseLowLevelLatencyParameters            (0x01)
#define paWinMmeUseMultipleDevices                      (0x02)
#define paWinMmeUseChannelMask                          (0x04)

#define paWinMmeDontThrottleOverloadedProcessingThread  (0x08)

#define paWinMmeWaveFormatDolbyAc3Spdif                 (0x10)
#define paWinMmeWaveFormatWmaSpdif                      (0x20)

typedef struct PaWinMmeDeviceAndChannelCount{
    PaDeviceIndex device;
    int channelCount;
}PaWinMmeDeviceAndChannelCount;

typedef struct PaWinMmeStreamInfo{
    unsigned long size;
    PaHostApiTypeId hostApiType;
    unsigned long version;

    unsigned long flags;

    unsigned long framesPerBuffer;
    unsigned long bufferCount;

    PaWinMmeDeviceAndChannelCount *devices;
    unsigned long deviceCount;

    PaWinWaveFormatChannelMask channelMask;

}PaWinMmeStreamInfo;

int PaWinMME_GetStreamInputHandleCount( PaStream* stream );

HWAVEIN PaWinMME_GetStreamInputHandle( PaStream* stream, int handleIndex );

int PaWinMME_GetStreamOutputHandleCount( PaStream* stream );

HWAVEOUT PaWinMME_GetStreamOutputHandle( PaStream* stream, int handleIndex );

#ifdef __cplusplus
}
#endif

#endif
