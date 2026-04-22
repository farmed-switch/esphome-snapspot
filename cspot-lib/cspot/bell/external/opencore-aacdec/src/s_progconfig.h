

#ifndef S_PROGCONFIG_H
#define S_PROGCONFIG_H

#include "pv_audio_type_defs.h"
#include "s_mixdown.h"
#include "s_elelist.h"

typedef struct
{
    Int     profile;
    Int     sampling_rate_idx;
    EleList front;
    EleList side;
    EleList back;
    EleList lfe;
    EleList data;
    EleList coupling;
    MIXdown mono_mix;
    MIXdown stereo_mix;
    MIXdown matrix_mix;

    Char    comments[(1<<LEN_PC_COMM)+1];

    Int32   buffer_fullness;
    Bool    file_is_adts;
    Int32   headerless_frames;
    Int32   frame_length;
    Int32   CRC_absent;
    UInt32  CRC_check;

} ProgConfig;

#endif
