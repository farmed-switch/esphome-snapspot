

#ifndef BIT_REVERSAL_SWAP_H
#define BIT_REVERSAL_SWAP_H

#include "pv_audio_type_defs.h"

extern const Int Index_64_a[];
extern const Int Index_64_b[];

extern const Int Index_512_a[];
extern const Int Index_512_b[];

void bit_reversal_swap(
    Int32        Data[],
    const Int *pIndex_a,
    const Int *pIndex_b);

#endif
