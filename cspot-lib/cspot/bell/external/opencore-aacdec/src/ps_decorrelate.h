

#ifndef PS_DECORRELATE_H
#define PS_DECORRELATE_H

#include "pv_audio_type_defs.h"
#include "s_ps_dec.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void ps_decorrelate(STRUCT_PS_DEC *h_ps_dec,
    Int32 *rIntBufferLeft,
    Int32 *iIntBufferLeft,
    Int32 *rIntBufferRight,
    Int32 *iIntBufferRight,
    Int32 scratch_mem[]);

#ifdef __cplusplus
}
#endif

#endif
