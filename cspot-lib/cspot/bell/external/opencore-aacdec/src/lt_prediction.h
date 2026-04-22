

#ifndef _LT_PREDICTION_H
#define _LT_PREDICTION_H

#include "block.h"
#include "ltp_common.h"
#include "ibstream.h"
#include "lt_decode.h"
#include "s_frameinfo.h"
#include "window_block.h"

void init_lt_pred(LT_PRED_STATUS * lt_status);

void lt_predict(
    Int                  object,
    FrameInfo           *pFrameInfo,
    WINDOW_SEQUENCE      win_seq,
    Wnd_Shape           *pWin_shape,
    LT_PRED_STATUS  *pLt_status,
    Real                *pPredicted_samples,
    Real                *pOverlap_buffer,
    Real                *pCurrent_frame_copy,
    Real                 current_frame[]);

short double_to_int(double sig_in);

#endif
