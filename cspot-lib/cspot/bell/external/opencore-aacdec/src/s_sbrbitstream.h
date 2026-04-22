

#ifndef S_SBRBITSTREAM_H
#define S_SBRBITSTREAM_H

#include    "s_sbr_element_stream.h"
#include    "s_sbr_channel.h"

typedef struct
{
    Int32 NrElements;
    Int32 NrElementsCore;
    SBR_ELEMENT_STREAM sbrElement[MAXNRELEMENTS];
}
SBRBITSTREAM;

#endif

