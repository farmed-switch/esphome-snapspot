

#ifndef TNS_INV_FILTER_H
#define TNS_INV_FILTER_H

#include "pv_audio_type_defs.h"

void tns_inv_filter(
    Int32 coef[],
    const Int   num_coef,
    const Int   inc,
    const Int32 lpc[],
    const Int   lpc_qformat,
    const Int   order,
    Int32 scratch_memory[]);

#endif

