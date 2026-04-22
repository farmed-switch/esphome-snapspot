

#ifndef SBR_DEC_H
#define SBR_DEC_H

#include    "s_sbr_frame_data.h"
#include    "pv_audio_type_defs.h"
#include    "s_patch.h"
#include    "e_blockswitching.h"
#include    "s_sbr_dec.h"
#include    "s_tdec_int_file.h"
#include    "config.h"
#ifdef PARAMETRICSTEREO
#include    "s_ps_dec.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    void sbr_dec(Int16 *inPcmData,
    Int16 *ftimeOutPtr,
    SBR_FRAME_DATA * hFrameData,
    Int32 applyProcessing,
    SBR_DEC *sbrDec,
#ifdef HQ_SBR
#ifdef PARAMETRICSTEREO
    Int16 * ftimeOutPtrPS,
    HANDLE_PS_DEC hParametricStereoDec,
#endif
#endif
    tDec_Int_File  *pVars);

#ifdef __cplusplus
}
#endif

#endif

