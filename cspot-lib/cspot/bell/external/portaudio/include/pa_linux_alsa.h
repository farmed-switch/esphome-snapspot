#ifndef PA_LINUX_ALSA_H
#define PA_LINUX_ALSA_H

#include "portaudio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PaAlsaStreamInfo
{
    unsigned long size;
    PaHostApiTypeId hostApiType;
    unsigned long version;

    const char *deviceString;
}
PaAlsaStreamInfo;

void PaAlsa_InitializeStreamInfo( PaAlsaStreamInfo *info );

void PaAlsa_EnableRealtimeScheduling( PaStream *s, int enable );

#if 0
void PaAlsa_EnableWatchdog( PaStream *s, int enable );
#endif

PaError PaAlsa_GetStreamInputCard( PaStream *s, int *card );

PaError PaAlsa_GetStreamOutputCard( PaStream *s, int *card );

PaError PaAlsa_SetNumPeriods( int numPeriods );

PaError PaAlsa_SetRetriesBusy( int retries );

void PaAlsa_SetLibraryPathName( const char *pathName );

#ifdef __cplusplus
}
#endif

#endif
