

#ifndef E_PROG_CONFIG_CONST_H
#define E_PROG_CONFIG_CONST_H

typedef enum
{

    Main_Profile    = 0,
    LC_Profile      = 1,

    Fs_48       = 3,
    Fs_44       = 4,
    Fs_32       = 5,

    LEN_PROFILE     = 2,
    LEN_SAMP_IDX    = 4,
    LEN_NUM_ELE     = 4,
    LEN_NUM_LFE     = 2,
    LEN_NUM_DAT     = 3,
    LEN_NUM_CCE     = 4,
    LEN_MIX_PRES    = 1,
    LEN_MMIX_IDX    = 2,
    LEN_PSUR_ENAB   = 1,
    LEN_ELE_IS_CPE  = 1,
    LEN_IND_SW_CCE  = 1,
    LEN_COMMENT_BYTES   = 8

} eProgConfigConst;

#endif

