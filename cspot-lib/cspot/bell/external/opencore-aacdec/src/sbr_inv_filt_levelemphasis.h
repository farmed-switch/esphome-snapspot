

#ifndef SBR_INV_FILT_LEVELEMPHASIS_H
#define SBR_INV_FILT_LEVELEMPHASIS_H

#include    "pv_audio_type_defs.h"
#include    "e_invf_mode.h"
#include    "sbr_generate_high_freq.h"

void sbr_inv_filt_levelemphasis(INVF_MODE *invFiltMode,
                                INVF_MODE *prevInvFiltMode,
                                Int32  nNfb,
                                Int32  BwVector[MAX_NUM_PATCHES],
                                Int32  BwVectorOld[MAX_NUM_PATCHES]);

#endif

