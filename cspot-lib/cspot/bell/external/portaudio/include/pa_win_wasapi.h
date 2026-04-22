#ifndef PA_WIN_WASAPI_H
#define PA_WIN_WASAPI_H

#include "portaudio.h"
#include "pa_win_waveformat.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum PaWasapiFlags
{

    paWinWasapiExclusive                = (1 << 0),

    paWinWasapiRedirectHostProcessor    = (1 << 1),

    paWinWasapiUseChannelMask           = (1 << 2),

    paWinWasapiPolling                  = (1 << 3),

    paWinWasapiThreadPriority           = (1 << 4),

    paWinWasapiExplicitSampleFormat     = (1 << 5),

    paWinWasapiAutoConvert              = (1 << 6)
}
PaWasapiFlags;
#define paWinWasapiExclusive             (paWinWasapiExclusive)
#define paWinWasapiRedirectHostProcessor (paWinWasapiRedirectHostProcessor)
#define paWinWasapiUseChannelMask        (paWinWasapiUseChannelMask)
#define paWinWasapiPolling               (paWinWasapiPolling)
#define paWinWasapiThreadPriority        (paWinWasapiThreadPriority)
#define paWinWasapiExplicitSampleFormat  (paWinWasapiExplicitSampleFormat)
#define paWinWasapiAutoConvert           (paWinWasapiAutoConvert)

typedef enum PaWasapiStreamState
{

    paWasapiStreamStateError         = (1 << 0),

    paWasapiStreamStateThreadPrepare = (1 << 1),

    paWasapiStreamStateThreadStart   = (1 << 2),

    paWasapiStreamStateThreadStop    = (1 << 3)
}
PaWasapiStreamState;
#define paWasapiStreamStateError         (paWasapiStreamStateError)
#define paWasapiStreamStateThreadPrepare (paWasapiStreamStateThreadPrepare)
#define paWasapiStreamStateThreadStart   (paWasapiStreamStateThreadStart)
#define paWasapiStreamStateThreadStop    (paWasapiStreamStateThreadStop)

typedef void (*PaWasapiHostProcessorCallback) (void *inputBuffer, long inputFrames,
    void *outputBuffer, long outputFrames, void *userData);

typedef void (*PaWasapiStreamStateCallback) (PaStream *pStream, unsigned int stateFlags,
    unsigned int errorId, void *pUserData);

typedef enum PaWasapiDeviceRole
{
    eRoleRemoteNetworkDevice = 0,
    eRoleSpeakers,
    eRoleLineLevel,
    eRoleHeadphones,
    eRoleMicrophone,
    eRoleHeadset,
    eRoleHandset,
    eRoleUnknownDigitalPassthrough,
    eRoleSPDIF,
    eRoleHDMI,
    eRoleUnknownFormFactor
}
PaWasapiDeviceRole;

typedef enum PaWasapiJackConnectionType
{
    eJackConnTypeUnknown,
    eJackConnType3Point5mm,
    eJackConnTypeQuarter,
    eJackConnTypeAtapiInternal,
    eJackConnTypeRCA,
    eJackConnTypeOptical,
    eJackConnTypeOtherDigital,
    eJackConnTypeOtherAnalog,
    eJackConnTypeMultichannelAnalogDIN,
    eJackConnTypeXlrProfessional,
    eJackConnTypeRJ11Modem,
    eJackConnTypeCombination
}
PaWasapiJackConnectionType;

typedef enum PaWasapiJackGeoLocation
{
    eJackGeoLocUnk = 0,
    eJackGeoLocRear = 0x1,
    eJackGeoLocFront,
    eJackGeoLocLeft,
    eJackGeoLocRight,
    eJackGeoLocTop,
    eJackGeoLocBottom,
    eJackGeoLocRearPanel,
    eJackGeoLocRiser,
    eJackGeoLocInsideMobileLid,
    eJackGeoLocDrivebay,
    eJackGeoLocHDMI,
    eJackGeoLocOutsideMobileLid,
    eJackGeoLocATAPI,
    eJackGeoLocReserved5,
    eJackGeoLocReserved6,
}
PaWasapiJackGeoLocation;

typedef enum PaWasapiJackGenLocation
{
    eJackGenLocPrimaryBox = 0,
    eJackGenLocInternal,
    eJackGenLocSeparate,
    eJackGenLocOther
}
PaWasapiJackGenLocation;

typedef enum PaWasapiJackPortConnection
{
    eJackPortConnJack = 0,
    eJackPortConnIntegratedDevice,
    eJackPortConnBothIntegratedAndJack,
    eJackPortConnUnknown
}
PaWasapiJackPortConnection;

typedef enum PaWasapiThreadPriority
{
    eThreadPriorityNone = 0,
    eThreadPriorityAudio,
    eThreadPriorityCapture,
    eThreadPriorityDistribution,
    eThreadPriorityGames,
    eThreadPriorityPlayback,
    eThreadPriorityProAudio,
    eThreadPriorityWindowManager
}
PaWasapiThreadPriority;

typedef struct PaWasapiJackDescription
{
    unsigned long              channelMapping;
    unsigned long              color;
    PaWasapiJackConnectionType connectionType;
    PaWasapiJackGeoLocation    geoLocation;
    PaWasapiJackGenLocation    genLocation;
    PaWasapiJackPortConnection portConnection;
    unsigned int               isConnected;
}
PaWasapiJackDescription;

typedef enum PaWasapiStreamCategory
{
    eAudioCategoryOther           = 0,
    eAudioCategoryCommunications  = 3,
    eAudioCategoryAlerts          = 4,
    eAudioCategorySoundEffects    = 5,
    eAudioCategoryGameEffects     = 6,
    eAudioCategoryGameMedia       = 7,
    eAudioCategoryGameChat        = 8,
    eAudioCategorySpeech          = 9,
    eAudioCategoryMovie           = 10,
    eAudioCategoryMedia           = 11
}
PaWasapiStreamCategory;

typedef enum PaWasapiStreamOption
{
    eStreamOptionNone        = 0,
    eStreamOptionRaw         = 1,
    eStreamOptionMatchFormat = 2
}
PaWasapiStreamOption;

typedef struct PaWasapiStreamInfo
{
    unsigned long size;
    PaHostApiTypeId hostApiType;
    unsigned long version;

    unsigned long flags;

    PaWinWaveFormatChannelMask channelMask;

    PaWasapiHostProcessorCallback hostProcessorOutput;
    PaWasapiHostProcessorCallback hostProcessorInput;

    PaWasapiThreadPriority threadPriority;

    PaWasapiStreamCategory streamCategory;

    PaWasapiStreamOption streamOption;
}
PaWasapiStreamInfo;

PaError PaWasapi_GetAudioClient( PaStream *pStream, void **pAudioClient, int bOutput );

PaError PaWasapi_UpdateDeviceList();

int PaWasapi_GetDeviceCurrentFormat( PaStream *pStream, void *pFormat, unsigned int formatSize, int bOutput );

int PaWasapi_GetDeviceDefaultFormat( void *pFormat, unsigned int formatSize, PaDeviceIndex device );

int PaWasapi_GetDeviceMixFormat( void *pFormat, unsigned int formatSize, PaDeviceIndex device );

int  PaWasapi_GetDeviceRole( PaDeviceIndex device );

PaError PaWasapi_GetIMMDevice( PaDeviceIndex device, void **pIMMDevice );

int PaWasapi_IsLoopback( PaDeviceIndex device );

PaError PaWasapi_ThreadPriorityBoost( void **pTask, PaWasapiThreadPriority priorityClass );

PaError PaWasapi_ThreadPriorityRevert( void *pTask );

PaError PaWasapi_GetFramesPerHostBuffer( PaStream *pStream, unsigned int *pInput, unsigned int *pOutput );

PaError PaWasapi_GetJackCount( PaDeviceIndex device, int *pJackCount );

PaError PaWasapi_GetJackDescription( PaDeviceIndex device, int jackIndex, PaWasapiJackDescription *pJackDescription );

PaError PaWasapi_SetStreamStateHandler( PaStream *pStream, PaWasapiStreamStateCallback fnStateHandler, void *pUserData );

PaError PaWasapiWinrt_SetDefaultDeviceId( const unsigned short *pId, int bOutput );

PaError PaWasapiWinrt_PopulateDeviceList( const unsigned short **pId, const unsigned short **pName,
    const PaWasapiDeviceRole *pRole, unsigned int count, int bOutput );

#ifdef __cplusplus
}
#endif

#endif
