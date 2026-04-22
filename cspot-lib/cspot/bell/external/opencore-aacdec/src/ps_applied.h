

#ifndef PS_APPLIED_H
#define PS_APPLIED_H

#include "pv_audio_type_defs.h"
#include "s_ps_dec.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void ps_applied(STRUCT_PS_DEC *h_ps_dec,
    Int32 rIntBufferLeft[][64],
    Int32 iIntBufferLeft[][64],
    Int32 *rIntBufferRight,
    Int32 *iIntBufferRight,
    Int32 scratch_mem[],
    Int32 band);

#ifdef __cplusplus
}
#endif

#endif
