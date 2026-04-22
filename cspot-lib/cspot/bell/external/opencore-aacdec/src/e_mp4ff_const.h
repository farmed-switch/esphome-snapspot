

#ifndef E_MP4FF_CONST_H
#define E_MP4FF_CONST_H

#include    "pv_audio_type_defs.h"

typedef enum
{
    LEN_OBJ_TYPE = 5,
    LEN_SAMP_RATE_IDX = 4,
    LEN_SAMP_RATE   = 24,
    LEN_CHAN_CONFIG = 4,
    LEN_SYNC_EXTENSION_TYPE = 11,
    LEN_FRAME_LEN_FLAG = 1,
    LEN_DEPEND_ON_CORE = 1,
    LEN_CORE_DELAY = 14,
    LEN_EXT_FLAG = 1,
    LEN_EP_CONFIG = 2,
    LEN_LAYER_NUM = 3,
    LEN_SUB_FRAME = 5,
    LEN_LAYER_LEN = 11,
    LEN_SECT_RES_FLAG = 1,
    LEN_SCF_RES_FLAG = 1,
    LEN_SPEC_RES_FLAG = 1,
    LEN_EXT_FLAG3 = 1
} eMP4FF_const;

#endif

