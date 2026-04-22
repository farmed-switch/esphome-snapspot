

#ifndef SBR_READ_DATA
#define SBR_READ_DATA

#include "e_sbr_error.h"
#include "s_sbr_channel.h"
#include "s_sbrbitstream.h"
#include "sbr_dec.h"

#ifdef __cplusplus
extern "C"
{
#endif

    SBR_ERROR sbr_read_data(SBRDECODER_DATA * self,
    SBR_DEC * sbrDec,
    SBRBITSTREAM *stream);

#ifdef __cplusplus
}
#endif

#endif

