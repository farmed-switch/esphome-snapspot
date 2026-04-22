#ifndef PA_WIN_WDMKS_H
#define PA_WIN_WDMKS_H

#include "portaudio.h"

#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum PaWinWDMKSFlags
    {

        paWinWDMKSOverrideFramesize   = (1 << 0),

        paWinWDMKSUseGivenChannelMask = (1 << 1),

    } PaWinWDMKSFlags;

    typedef struct PaWinWDMKSInfo{
        unsigned long size;
        PaHostApiTypeId hostApiType;
        unsigned long version;

        unsigned long flags;

        unsigned noOfPackets;

        unsigned channelMask;
    } PaWinWDMKSInfo;

    typedef enum PaWDMKSType
    {
        Type_kNotUsed,
        Type_kWaveCyclic,
        Type_kWaveRT,
        Type_kCnt,
    } PaWDMKSType;

    typedef enum PaWDMKSSubType
    {
        SubType_kUnknown,
        SubType_kNotification,
        SubType_kPolled,
        SubType_kCnt,
    } PaWDMKSSubType;

    typedef struct PaWinWDMKSDeviceInfo {
        wchar_t filterPath[MAX_PATH];
        wchar_t topologyPath[MAX_PATH];
        PaWDMKSType streamingType;
        GUID deviceProductGuid;
    } PaWinWDMKSDeviceInfo;

    typedef struct PaWDMKSDirectionSpecificStreamInfo
    {
        PaDeviceIndex device;
        unsigned channels;
        unsigned framesPerHostBuffer;
        int endpointPinId;
        int muxNodeId;
        PaWDMKSSubType streamingSubType;
    } PaWDMKSDirectionSpecificStreamInfo;

    typedef struct PaWDMKSSpecificStreamInfo {
        PaWDMKSDirectionSpecificStreamInfo input;
        PaWDMKSDirectionSpecificStreamInfo output;
    } PaWDMKSSpecificStreamInfo;

#ifdef __cplusplus
}
#endif

#endif
