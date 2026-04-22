

#ifndef S_CH_INFO_H
#define S_CH_INFO_H

#include "pv_audio_type_defs.h"

typedef struct
{

    Int tag;
    Int cpe;

    Int is_present;
    Int ncch;

    Char *fext;

} Ch_Info;

#endif

