

#ifndef IBSTREAM_H
#define IBSTREAM_H

#include    "pv_audio_type_defs.h"
#include    "s_bits.h"
#include    "getbits.h"

#define INBUF_ARRAY_INDEX_SHIFT  (3)
#define INBUF_BIT_WIDTH         (1<<(INBUF_ARRAY_INDEX_SHIFT))
#define INBUF_BIT_MODULO_MASK   ((INBUF_BIT_WIDTH)-1)

#define MAX_GETBITS             (25)

#ifdef __cplusplus
extern "C"
{
#endif

    void byte_align(
        BITS  *pInputStream);

#ifdef __cplusplus
}
#endif

#endif

