

#ifndef GET_ADIF_HEADER_H
#define GET_ADIF_HEADER_H

#include "pv_audio_type_defs.h"
#include "s_tdec_int_file.h"

#define CONSTANT_RATE_BITSTREAM  (0)
#define VARIABLE_RATE_BITSTREAM  (1)

Int get_adif_header(
    tDec_Int_File *pVars,
    ProgConfig    *pScratchPCE);

#endif

