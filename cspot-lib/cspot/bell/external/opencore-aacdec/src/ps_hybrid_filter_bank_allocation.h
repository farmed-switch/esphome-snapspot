

#ifndef PS_HYBRID_FILTER_BANK_ALLOCATION_H
#define PS_HYBRID_FILTER_BANK_ALLOCATION_H

#include    "pv_audio_type_defs.h"
#include    "s_hybrid.h"

#ifdef __cplusplus
extern "C"
{
#endif

    Int32 ps_hybrid_filter_bank_allocation(HYBRID **phHybrid,
    Int32 noBands,
    const Int32 *pResolution,
    Int32 **pPtr);

#ifdef __cplusplus
}
#endif

#endif
