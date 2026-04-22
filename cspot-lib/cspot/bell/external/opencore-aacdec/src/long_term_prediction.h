

#ifndef LONG_TERM_PREDICTION_H
#define LONG_TERM_PREDICTION_H

#include "pv_audio_type_defs.h"
#include "e_window_sequence.h"

#define LTP_Q_FORMAT    (15)

#ifdef __cplusplus
extern "C"
{
#endif

    Int long_term_prediction(
        WINDOW_SEQUENCE     win_seq,
        const Int           weight_index,
        const Int           delay[],
        const Int16         buffer[],
        const Int           buffer_offset,
        const Int32         time_quant[],
        Int32               predicted_samples[],
        const Int           frame_length);

#ifdef __cplusplus
}
#endif

#endif

