

#ifndef MDST_H
#define MDST_H

#include "pv_audio_type_defs.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void mdst_32(Int32 vec[], Int32 scratch_mem[]);

    void  dct_32(Int32 vec[]);

    void mdct_32(Int32 vec[]);

#ifdef __cplusplus
}
#endif

#endif
