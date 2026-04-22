

#ifndef SBR_ALIASING_REDUCTION_H
#define SBR_ALIASING_REDUCTION_H

#include "pv_audio_type_defs.h"

void sbr_aliasing_reduction(Int32 *degreeAlias,
                            Int32  * nrg_gain_man,
                            Int32  * nrg_gain_exp,
                            Int32  * nrg_est_man,
                            Int32  * nrg_est_exp,
                            Int32  * dontUseTheseGainValues,
                            Int32    noSubbands,
                            Int32    lowSubband,
                            Int32  sqrt_cache[][4],
                            Int32 * groupVector);

#endif

