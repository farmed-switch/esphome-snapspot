

#ifndef PS_INIT_STEREO_MIXING_H
#define PS_INIT_STEREO_MIXING_H

#include "pv_audio_type_defs.h"
#include "s_ps_dec.h"

#ifdef __cplusplus
extern "C"
{
#endif

    Int32 ps_init_stereo_mixing(STRUCT_PS_DEC *pms,
    Int32 env,
    Int32 usb);

#ifdef __cplusplus
}
#endif

#endif
