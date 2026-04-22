

#ifndef DST32_H
#define DST32_H

#include "pv_audio_type_defs.h"

#ifdef __cplusplus
extern "C"
{
#endif

    extern const Int32 CosTable_16[];

    void dst_32(Int32 vec[], Int32 scratch_mem[]);

#ifdef __cplusplus
}
#endif

#endif
