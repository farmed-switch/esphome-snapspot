

#ifndef INV_LONG_COMPLEX_ROT_H
#define INV_LONG_COMPLEX_ROT_H

#include "pv_audio_type_defs.h"

#define INV_LONG_CX_ROT_LENGTH          256
#define TWICE_INV_LONG_CX_ROT_LENGTH (INV_LONG_CX_ROT_LENGTH<<1)

#ifdef __cplusplus
extern "C"
{
#endif

    Int inv_long_complex_rot(
        Int32 *Data,
        Int32  max);

#ifdef __cplusplus
}
#endif

#endif
