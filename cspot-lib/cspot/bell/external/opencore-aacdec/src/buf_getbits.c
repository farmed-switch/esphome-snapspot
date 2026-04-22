

#include "config.h"

#ifdef AAC_PLUS

#include    "buf_getbits.h"

UInt32 buf_getbits(BIT_BUFFER * hBitBuf, Int32 n)
{

    if (hBitBuf->buffered_bits <= 16)
    {
        hBitBuf->buffer_word    = (hBitBuf->buffer_word << 16) | (*(hBitBuf->char_ptr++) << 8);
        hBitBuf->buffer_word   |= *(hBitBuf->char_ptr++);
        hBitBuf->buffered_bits += 16;
    }

    hBitBuf->buffered_bits -= n;
    hBitBuf->nrBitsRead    += n;

    return ((hBitBuf->buffer_word >> hBitBuf->buffered_bits) & ((1 << n) - 1));

}

UInt32 buf_get_1bit(BIT_BUFFER * hBitBuf)
{

    if (hBitBuf->buffered_bits <= 16)
    {
        hBitBuf->buffer_word    = (hBitBuf->buffer_word << 16) | (*(hBitBuf->char_ptr++) << 8);
        hBitBuf->buffer_word   |= *(hBitBuf->char_ptr++);
        hBitBuf->buffered_bits += 16;
    }

    hBitBuf->buffered_bits--;
    hBitBuf->nrBitsRead++;

    return ((hBitBuf->buffer_word >> hBitBuf->buffered_bits) & 1);

}

#endif
