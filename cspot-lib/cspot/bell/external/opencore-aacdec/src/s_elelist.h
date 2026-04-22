

#ifndef S_ELELIST_H
#define S_ELELIST_H

#include "pv_audio_type_defs.h"
#include "e_rawbitstreamconst.h"

typedef struct
{
    Int num_ele;
    Int ele_is_cpe[(1<<LEN_TAG)];
    Int ele_tag[(1<<LEN_TAG)];
} EleList;

#endif
