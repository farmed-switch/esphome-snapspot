

#ifndef FIND_ADTS_SYNCWORD_H
#define FIND_ADTS_SYNCWORD_H

#include "s_bits.h"

Int find_adts_syncword(
    UInt32 *pSyncword,
    BITS   *pInputStream,
    Int     syncword_length,
    UInt32  syncword_mask);

#endif
