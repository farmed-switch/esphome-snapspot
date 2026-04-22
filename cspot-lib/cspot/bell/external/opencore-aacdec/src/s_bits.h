

#ifndef  S_BITS_H
#define  S_BITS_H

#include "pv_audio_type_defs.h"

typedef struct
{
    UChar    *pBuffer;
    UInt      usedBits;
    UInt      availableBits;
    UInt      inputBufferCurrentLength;
    Int      byteAlignOffset;
} BITS;

#endif

