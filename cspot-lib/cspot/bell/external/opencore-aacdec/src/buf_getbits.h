

#ifndef BUF_GETBITS_H
#define BUF_GETBITS_H

#include "pv_audio_type_defs.h"
#include "s_bit_buffer.h"

#ifdef __cplusplus
extern "C"
{
#endif

    UInt32 buf_getbits(BIT_BUFFER * hBitBuf, Int32 n);

    UInt32 buf_get_1bit(BIT_BUFFER * hBitBuf);

#ifdef __cplusplus
}
#endif

#endif

