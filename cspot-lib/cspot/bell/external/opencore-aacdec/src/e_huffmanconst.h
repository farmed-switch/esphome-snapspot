

#ifndef E_HUFFMAN_CONST_H
#define E_HUFFMAN_CONST_H

typedef enum
{

    HUF1SGN     = 1,
    HUF2SGN     = 1,
    HUF3SGN     = 0,
    HUF4SGN     = 0,
    HUF5SGN     = 1,
    HUF6SGN     = 1,
    HUF7SGN     = 0,
    HUF8SGN     = 0,
    HUF9SGN     = 0,
    HUF10SGN        = 0,
    HUF11SGN        = 0,

    ZERO_HCB        = 0,
    BY4BOOKS        = 4,
    ESCBOOK     = 11,
    NSPECBOOKS      = ESCBOOK + 1,
    BOOKSCL     = NSPECBOOKS,
    NBOOKS      = NSPECBOOKS + 1,
    INTENSITY_HCB2  = 14,
    INTENSITY_HCB   = 15,
    NOISE_HCB       = 13,
    NOISE_HCB2      = 113,

    NOISE_PCM_BITS      = 9,
    NOISE_PCM_OFFSET    = (1 << (NOISE_PCM_BITS - 1)),

    NOISE_OFFSET        = 90,

    LONG_SECT_BITS  = 5,
    SHORT_SECT_BITS = 3
} eHuffmanConst;

#endif

