

#ifndef CALC_SBR_SYNFILTERBANK_H
#define CALC_SBR_SYNFILTERBANK_H

#include "pv_audio_type_defs.h"
#include "config.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define N  14

#define ROUND_SYNFIL  (32768 + 4096)

    void calc_sbr_synfilterbank_LC(Int32 * Sr,
    Int16 * timeSig,
    Int16   V[1280],
    Bool bDownSampleSBR);

#ifdef HQ_SBR

    void calc_sbr_synfilterbank(Int32 * Sr,
                                Int32 * Si,
                                Int16 * timeSig,
                                Int16   V[1280],
                                Bool bDownSampleSBR);

#endif

#ifdef __cplusplus
}
#endif

#endif

