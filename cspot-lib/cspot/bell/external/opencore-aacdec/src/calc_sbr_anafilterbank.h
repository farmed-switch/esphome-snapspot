

#ifndef CALC_SBR_ANAFILTERBANK_H
#define CALC_SBR_ANAFILTERBANK_H

#include "pv_audio_type_defs.h"
#include "config.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define ROUND_ANAFIL     0

#define ROUND_ANAFIL_LC  (0)

    void calc_sbr_anafilterbank_LC(Int32 * Sr,
    Int16 * X,
    Int32 scratch_mem[][64],
    Int32 maxBand);

#ifdef HQ_SBR

    void calc_sbr_anafilterbank(Int32 * Sr,
                                Int32 * Si,
                                Int16 * X,
                                Int32 scratch_mem[][64],
                                Int32 maxBand);

#endif

#ifdef __cplusplus
}
#endif

#endif
