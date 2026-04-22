

#ifndef LONG_TERM_SYNTHESIS_H
#define LONG_TERM_SYNTHESIS_H

#include "pv_audio_type_defs.h"
#include "e_window_sequence.h"

void long_term_synthesis(
    WINDOW_SEQUENCE     win_seq,
    Int                 sfb_per_win,
    Int16               win_sfb_top[],
    Int                 win_prediction_used[],
    Int                 sfb_prediction_used[],
    Int32               current_frame[],
    Int                 q_format[],
    Int32               predicted_spectral[],
    Int                 pred_q_format,
    Int                 coef_per_win,
    Int                 short_window_num,
    Int                 reconstruct_sfb_num);

#endif

