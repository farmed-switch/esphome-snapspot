

#ifndef E_ADIF_CONST_H
#define E_ADIF_CONST_H

typedef enum
{

    LEN_ADIF_ID     = (32 / 8),
    LEN_COPYRT_PRES = 1,
    LEN_COPYRT_ID   = (72 / 8),
    LEN_ORIG        = 1,
    LEN_HOME        = 1,
    LEN_BS_TYPE     = 1,
    LEN_BIT_RATE    = 23,
    LEN_NUM_PCE     = 4,
    LEN_ADIF_BF     = 20

} eADIF_Const;

#endif
