

#include "pv_audio_type_defs.h"
#include "s_tdec_int_file.h"
#include "e_progconfigconst.h"

#include "huffman.h"
#include "aac_mem_funcs.h"
#include "pvmp4audiodecoder_api.h"
#include "s_tdec_int_chan.h"
#include "sfb.h"
#include "config.h"

OSCL_EXPORT_REF Int PVMP4AudioDecoderInitLibrary(
    tPVMP4AudioDecoderExternal  *pExt,
    void                        *pMem)
{
    tDec_Int_File *pVars;

    pVars = (tDec_Int_File *)pMem;

    pv_memset(
        pVars,
        0,
        sizeof(tDec_Int_File));

    pVars->perChan[0].fxpCoef = pVars->fxpCoef[0];
    pVars->perChan[1].fxpCoef = pVars->fxpCoef[1];

    pVars->perChan[0].pShareWfxpCoef = (per_chan_share_w_fxpCoef *)
                                       & (pVars->perChan[0].fxpCoef[1024]);

    pVars->perChan[1].pShareWfxpCoef = (per_chan_share_w_fxpCoef *)
                                       & (pVars->perChan[1].fxpCoef[1024]);

    pVars->current_program = -1;
    pVars->mc_info.sampling_rate_idx = Fs_44;

    pVars->frameLength = LONG_WINDOW;

    pVars->winmap[ONLY_LONG_SEQUENCE]     = &pVars->longFrameInfo;
    pVars->winmap[LONG_START_SEQUENCE]    = &pVars->longFrameInfo;
    pVars->winmap[EIGHT_SHORT_SEQUENCE]   = &pVars->shortFrameInfo;
    pVars->winmap[LONG_STOP_SEQUENCE]     = &pVars->longFrameInfo;

    infoinit(
        pVars->mc_info.sampling_rate_idx,
        (FrameInfo   **)pVars->winmap,
        pVars->SFBWidth128);

    pExt->bitRate = 0;
    pExt->encodedChannels = 0;
    pExt->samplingRate = 0;
    pExt->aacPlusUpsamplingFactor = 1;
    pVars->aacPlusEnabled = pExt->aacPlusEnabled;

#if defined(AAC_PLUS)
    pVars->sbrDecoderData.setStreamType = 1;
#endif

    pExt->inputBufferUsedLength = 0;

    return (MP4AUDEC_SUCCESS);

}

OSCL_EXPORT_REF void PVMP4AudioDecoderDisableAacPlus(
    tPVMP4AudioDecoderExternal  *pExt,
    void                        *pMem)
{
    tDec_Int_File *pVars;

    pVars = (tDec_Int_File *)pMem;

    if ((pVars->aacPlusEnabled == TRUE) && (pExt->aacPlusEnabled == TRUE))
    {

        pVars->aacPlusEnabled = FALSE;
        pExt->aacPlusEnabled = FALSE;

#if defined(AAC_PLUS)
        pVars->mc_info.upsamplingFactor = 1;
        pVars->mc_info.psPresentFlag  = 0;
        pVars->mc_info.sbrPresentFlag = 0;
        pVars->prog_config.sampling_rate_idx += 3;
        pVars->sbrDecoderData.SbrChannel[0].syncState = SBR_NOT_INITIALIZED;
        pVars->sbrDecoderData.SbrChannel[1].syncState = SBR_NOT_INITIALIZED;

        pExt->samplingRate = samp_rate_info[pVars->prog_config.sampling_rate_idx].samp_rate;
        pExt->aacPlusUpsamplingFactor = 1;
#endif
    }
}

