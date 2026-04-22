

#ifndef GET_GA_SPECIFIC_CONFIG_H
#define GET_GA_SPECIFIC_CONFIG_H

#include    "pv_audio_type_defs.h"
#include    "s_tdec_int_file.h"
#include    "s_bits.h"

Int get_GA_specific_config(
    tDec_Int_File * const pVars,
    BITS    *pInputStream,
    UInt     channel_config,
    const tMP4AudioObjectType audioObjectType
);

#endif

