

#ifndef IDCT16_H
#define IDCT16_H

#include "pv_audio_type_defs.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void idct_16(Int32 vec[], Int32 scratch_mem[]);

#ifdef __cplusplus
}
#endif

#endif
