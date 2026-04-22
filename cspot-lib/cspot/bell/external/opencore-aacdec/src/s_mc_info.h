

#ifndef S_MC_INFO_H
#define S_MC_INFO_H

#include "pv_audio_type_defs.h"
#include "e_rawbitstreamconst.h"
#include "s_ch_info.h"
#include "chans.h"
#include "e_tmp4audioobjecttype.h"
#include "config.h"

typedef struct
{
    Int nch;
    Int nfsce;
    Int nfch;
    Int nsch;
    Int nbch;
    Int nlch;
    Int ncch;
    tMP4AudioObjectType audioObjectType;
    Int sampling_rate_idx;

    Int implicit_channeling;
    Int  upsamplingFactor;
#ifdef AAC_PLUS
    Bool bDownSampledSbr;
    Int HE_AAC_level;
#endif

    Int sbrPresentFlag;

    Int psPresentFlag;
    tMP4AudioObjectType ExtendedAudioObjectType;

    Ch_Info ch_info[Chans];
} MC_Info;

#endif

