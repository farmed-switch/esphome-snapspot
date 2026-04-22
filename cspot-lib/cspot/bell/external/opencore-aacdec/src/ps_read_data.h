

#ifndef PS_READ_DATA_H
#define PS_READ_DATA_H

#include "pv_audio_type_defs.h"
#include "s_ps_dec.h"

#define EXTENSION_ID_PS_CODING   2

#ifdef __cplusplus
extern "C"
{
#endif

    Int32 ps_read_data(STRUCT_PS_DEC *ps_dec,
    BIT_BUFFER * hBitBuf,
    Int32 nBitsLeft);

#ifdef __cplusplus
}
#endif

#endif
