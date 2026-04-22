

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_decode_huff_cw.h"
#include    "buf_getbits.h"

Int32 sbr_decode_huff_cw(SbrHuffman h,
                         BIT_BUFFER * hBitBuf)
{
    Int32 bits;
    Char index = 0;

    while (index >= 0)
    {
        bits = buf_get_1bit(hBitBuf);
        index = h[index][bits];
    }

    return((Int32)index + 64);
}

#endif
