

#ifndef S_ADIF_HEADER_H
#define S_ADIF_HEADER_H

#include "pv_audio_type_defs.h"
#include "e_adif_const.h"
#include "e_rawbitstreamconst.h"

typedef struct
{
    Char    adif_id[LEN_ADIF_ID+1];
    Int     copy_id_present;
    Char    copy_id[LEN_COPYRT_ID+1];
    Int     original_copy;
    Int     home;
    Int     bitstream_type;
    Int32   bitrate;
    Int     num_pce;
    Int     prog_tags[(1<<LEN_TAG)];
} ADIF_Header;

#endif

