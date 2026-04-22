

#ifndef S_SBR_CHANNEL_H
#define S_SBR_CHANNEL_H

#include    "s_sbr_frame_data.h"
#include    "e_sbr_sync_state.h"
#include    "config.h"
#ifdef PARAMETRICSTEREO
#include "s_ps_dec.h"

#endif

#define MAXNRELEMENTS 1
#define MAXNRSBRCHANNELS (MAXNRELEMENTS*2)

#ifdef PARAMETRICSTEREO
#define MAXNRQMFCHANNELS MAXNRSBRCHANNELS
#else
#define MAXNRQMFCHANNELS MAXNRSBRCHANNELS
#endif

typedef struct
{
    Int32 outFrameSize;
    SBR_SYNC_STATE syncState;
    SBR_FRAME_DATA frameData;

} SBR_CHANNEL;

typedef struct
{
    SBR_CHANNEL SbrChannel[MAXNRSBRCHANNELS];
    Int32 setStreamType;
#ifdef PARAMETRICSTEREO
    HANDLE_PS_DEC hParametricStereoDec;
    STRUCT_PS_DEC ParametricStereoDec;
#endif

} SBRDECODER_DATA;

#endif

