

#ifndef E_BLOCK_SWITCHING_H
#define E_BLOCK_SWITCHING_H

typedef enum
{

    LN          = 2048,
    SN          = 256,
    LN2         = LN / 2,
    SN2         = SN / 2,
    LN4         = LN / 4,
    SN4         = SN / 4,
    NSHORT      = LN / SN,
    MAX_SBK     = NSHORT,
    MAX_WIN     = MAX_SBK,

    ONLY_LONG_WINDOW    = 0,
    LONG_START_WINDOW,
    EIGHT_SHORT_WINDOW,
    LONG_STOP_WINDOW,
    NUM_WIN_SEQ,

    WLONG       = ONLY_LONG_WINDOW,
    WSTART,
    WSHORT,
    WSTOP,

    MAXBANDS        = 16 * NSHORT,
    MAXFAC      = 121,
    MIDFAC      = (MAXFAC - 1) / 2,
    SF_OFFSET       = 100
} eBlockSwitching;

#endif
