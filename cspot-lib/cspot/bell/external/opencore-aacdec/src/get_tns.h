

#ifndef GET_TNS_H
#define GET_TNS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "pv_audio_type_defs.h"
#include "s_frameinfo.h"
#include "s_mc_info.h"
#include "s_tns_frame_info.h"
#include "s_bits.h"
#include "e_window_sequence.h"

    void get_tns(
        const Int               max_bands,
        BITS            * const pInputStream,
        const WINDOW_SEQUENCE   wnd_seq,
        const FrameInfo * const pFrameInfo,
        const MC_Info   * const pMC_Info,
        TNS_frame_info  * const pTnsFrameInfo,
        Int32                   scratchTnsDecCoefMem[]);

#ifdef __cplusplus
}
#endif

#endif

