

#ifndef CHANS_H
#define CHANS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "pv_audio_type_defs.h"

#define ICChans 0
#define DCChans 0
#define XCChans 0
#define CChans  0

    enum
    {

        FChans  = 2,
        FCenter = 0,
        SChans  = 0,
        BChans  = 0,
        BCenter = 0,
        LChans  = 0,
        XChans  = 0,

        Chans   = FChans + SChans + BChans + LChans + XChans
    };

#ifdef __cplusplus
}
#endif

#endif

