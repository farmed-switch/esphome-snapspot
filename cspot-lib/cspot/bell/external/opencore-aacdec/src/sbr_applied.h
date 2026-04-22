

#ifndef SBR_APPLIED_H
#define SBR_APPLIED_H

#include "e_sbr_error.h"
#include "s_sbr_channel.h"
#include "s_sbrbitstream.h"
#include "sbr_dec.h"
#include "pv_audio_type_defs.h"
#include "s_tdec_int_file.h"

#define MAX_FRAME_SIZE  1024

#ifdef __cplusplus
extern "C"
{
#endif

    SBR_ERROR  sbr_applied(SBRDECODER_DATA * self,
    SBRBITSTREAM * stream,
    Int16 *ch_left,
    Int16 *ch_right,
    Int16 *timeData,
    SBR_DEC *sbrDec,
    tDec_Int_File  *pVars,
    Int32 numChannels);

#ifdef __cplusplus
}
#endif

#endif

