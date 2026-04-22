

#ifndef S_TDEC_INT_CHAN_H
#define S_TDEC_INT_CHAN_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "pv_audio_type_defs.h"
#include "e_rawbitstreamconst.h"
#include "s_tns_frame_info.h"
#include "s_wnd_shape.h"
#include "s_lt_pred_status.h"
#include "s_sectinfo.h"
#include "s_frameinfo.h"
#include "e_window_shape.h"
#include "e_window_sequence.h"
#include "window_block_fxp.h"
#include "config.h"

    typedef struct
    {
        TNS_frame_info       tns;

        FrameInfo            frameInfo;

        Int                  factors[MAXBANDS];
        Int                  cb_map[MAXBANDS];
        Int                  group[NSHORT];
        Int                  qFormat[MAXBANDS];

        Int                  max_sfb;
        LT_PRED_STATUS       lt_status;

    } per_chan_share_w_fxpCoef;

    typedef struct
    {
#ifdef AAC_PLUS
        Int16                ltp_buffer[LT_BLEN + 2*288];
#else
        Int16                ltp_buffer[LT_BLEN];
#endif

        Int32                time_quant[LONG_WINDOW];

        Int32                *fxpCoef;

        per_chan_share_w_fxpCoef * pShareWfxpCoef;

        Int32                abs_max_per_window[NUM_SHORT_WINDOWS];

        WINDOW_SEQUENCE      wnd;

        WINDOW_SHAPE         wnd_shape_prev_bk;
        WINDOW_SHAPE         wnd_shape_this_bk;

    } tDec_Int_Chan;

#ifdef __cplusplus
}
#endif

#endif

