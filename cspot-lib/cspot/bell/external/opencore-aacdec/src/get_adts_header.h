

#ifndef GET_ADTS_HEADER_H
#define GET_ADTS_HEADER_H

#include "pv_audio_type_defs.h"
#include "s_tdec_int_file.h"

Int get_adts_header(
    tDec_Int_File *pVars,
    UInt32        *pSyncword,
    Int           *pInvoke,
    Int            CorrectlyReadFramesCount);

#endif

