

#ifndef APPLY_TNS_H
#define APPLY_TNS_H

#include "pv_audio_type_defs.h"
#include "s_tns_frame_info.h"
#include "s_frameinfo.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void apply_tns(
        Int32                  coef[],
        Int                    q_format[],
        const FrameInfo      * const pFrameInfo,
        TNS_frame_info * const pTNS_frame_info,
        const Bool                   inverse_flag,
        Int32                  scratch_Int_buffer[]);

#ifdef __cplusplus
}
#endif

#endif

