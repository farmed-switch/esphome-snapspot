

#ifndef GET_ICS_INFO_H
#define GET_ICS_INFO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "pv_audio_type_defs.h"
#include "e_tmp4audioobjecttype.h"
#include "s_bits.h"
#include "e_window_sequence.h"
#include "e_window_shape.h"
#include "s_frameinfo.h"
#include "s_lt_pred_status.h"

    Int get_ics_info(
        const tMP4AudioObjectType  audioObjectType,
        BITS                      *pInputStream,
        const Bool                 common_window,
        WINDOW_SEQUENCE           *p_wnd,
        WINDOW_SHAPE              *pWindowShape,
        Int                        group[],
        Int                       *p_max_sfb,
        FrameInfo                 *p_winmap[],
        LT_PRED_STATUS            *pFirstLTPStatus,
        LT_PRED_STATUS            *pSecondLTPStatus);

#ifdef __cplusplus
}
#endif

#endif

