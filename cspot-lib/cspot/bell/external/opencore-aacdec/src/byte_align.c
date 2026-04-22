

#include "pv_audio_type_defs.h"
#include "s_bits.h"
#include "ibstream.h"

#define BYTE_ALIGN_MASK    ((UInt)(-8))

#define BYTE_ALIGN_ROUNDUP  7

void byte_align(
    BITS  *pInputStream)
{

    pInputStream->usedBits += BYTE_ALIGN_ROUNDUP;
    pInputStream->usedBits &= BYTE_ALIGN_MASK;

    return;
}

