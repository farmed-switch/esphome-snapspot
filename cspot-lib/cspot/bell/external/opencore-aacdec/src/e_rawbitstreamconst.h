

#ifndef E_RAW_BITSTREAM_CONST_H
#define E_RAW_BITSTREAM_CONST_H

typedef enum
{
    LEN_SE_ID       = 3,
    LEN_TAG     = 4,
    LEN_COM_WIN     = 1,
    LEN_ICS_RESERV  = 1,
    LEN_WIN_SEQ     = 2,
    LEN_WIN_SH      = 1,
    LEN_MAX_SFBL    = 6,
    LEN_MAX_SFBS    = 4,
    LEN_CB          = 4,
    LEN_SCL_PCM     = 8,
    LEN_PRED_PRES   = 1,
    LEN_PRED_RST    = 1,
    LEN_PRED_RSTGRP = 5,
    LEN_PRED_ENAB   = 1,
    LEN_MASK_PRES   = 2,
    LEN_MASK        = 1,
    LEN_PULSE_PRES  = 1,
    LEN_TNS_PRES    = 1,
    LEN_GAIN_PRES   = 1,

    LEN_PULSE_NPULSE    = 2,
    LEN_PULSE_ST_SFB    = 6,
    LEN_PULSE_POFF      = 5,
    LEN_PULSE_PAMP      = 4,
    NUM_PULSE_LINES     = 4,
    PULSE_OFFSET_AMP    = 4,

    LEN_IND_CCE_FLG = 1,
    LEN_NCC         = 3,
    LEN_IS_CPE      = 1,
    LEN_CC_LR       = 1,
    LEN_CC_DOM      = 1,
    LEN_CC_SGN      = 1,
    LEN_CCH_GES     = 2,
    LEN_CCH_CGP     = 1,

    LEN_D_ALIGN     = 1,
    LEN_D_CNT       = 8,
    LEN_D_ESC       = 8,
    LEN_F_CNT       = 4,
    LEN_F_ESC       = 8,
    LEN_BYTE        = 8,
    LEN_PAD_DATA    = 8,

    LEN_PC_COMM     = 9

} eRawBitstreamConst;

#endif
