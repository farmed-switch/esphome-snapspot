

#ifndef LT_DECODE_H
#define LT_DECODE_H

#include "pv_audio_type_defs.h"
#include "e_window_sequence.h"
#include "s_lt_pred_status.h"
#include "s_bits.h"

void lt_decode(
    const WINDOW_SEQUENCE win_type,
    BITS           *pInputStream,
    const Int             max_sfb,
    LT_PRED_STATUS *pLt_pred);

#endif

