

#ifndef SBR_DECODE_HUFF_CW_H
#define SBR_DECODE_HUFF_CW_H

#include    "pv_audio_type_defs.h"
#include    "s_bit_buffer.h"
#include    "s_huffman.h"

#ifdef __cplusplus
extern "C"
{
#endif

    Int32 sbr_decode_huff_cw(SbrHuffman h,
    BIT_BUFFER * hBitBuf);

#ifdef __cplusplus
}
#endif

#endif

