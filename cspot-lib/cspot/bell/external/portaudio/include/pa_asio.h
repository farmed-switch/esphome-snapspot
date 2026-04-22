#ifndef PA_ASIO_H
#define PA_ASIO_H

#include "portaudio.h"

#ifdef __cplusplus
extern "C"
{
#endif

PaError PaAsio_GetAvailableBufferSizes( PaDeviceIndex device,
        long *minBufferSizeFrames, long *maxBufferSizeFrames, long *preferredBufferSizeFrames, long *granularity );

#define PaAsio_GetAvailableLatencyValues PaAsio_GetAvailableBufferSizes

PaError PaAsio_ShowControlPanel( PaDeviceIndex device, void* systemSpecific );

PaError PaAsio_GetInputChannelName( PaDeviceIndex device, int channelIndex,
        const char** channelName );

PaError PaAsio_GetOutputChannelName( PaDeviceIndex device, int channelIndex,
        const char** channelName );

PaError PaAsio_SetStreamSampleRate( PaStream* stream, double sampleRate );

#define paAsioUseChannelSelectors      (0x01)

typedef struct PaAsioStreamInfo{
    unsigned long size;
    PaHostApiTypeId hostApiType;
    unsigned long version;

    unsigned long flags;

    int *channelSelectors;
}PaAsioStreamInfo;

#ifdef __cplusplus
}
#endif

#endif
