

#include "pv_audio_type_defs.h"

#include "s_tdec_int_chan.h"
#include "s_tdec_int_file.h"
#include "aac_mem_funcs.h"
#include "sfb.h"
#include "e_tmp4audioobjecttype.h"
#include "e_elementid.h"

#include "get_adif_header.h"
#include "get_adts_header.h"
#include "get_audio_specific_config.h"
#include "ibstream.h"

#include "huffman.h"
#include "get_prog_config.h"
#include "getfill.h"
#include "pns_left.h"

#include "apply_ms_synt.h"
#include "pns_intensity_right.h"
#include "q_normalize.h"
#include "long_term_prediction.h"
#include "long_term_synthesis.h"
#include "ltp_common_internal.h"
#include "apply_tns.h"

#include "window_block_fxp.h"

#include "pvmp4audiodecoder_api.h"
#include "get_dse.h"

#include "sbr_applied.h"
#include "sbr_open.h"
#include "get_sbr_bitstream.h"
#include "e_sbr_element_id.h"
#include "config.h"

#define LEFT (0)
#define RIGHT (1)

void InitSbrSynFilterbank(Bool bDownSampleSBR);

OSCL_EXPORT_REF Int PVMP4AudioDecodeFrame(
    tPVMP4AudioDecoderExternal  *pExt,
    void                        *pMem)
{
    Int            frameLength;
    Int            ch;
    Int            id_syn_ele;
    UInt           initialUsedBits;
    Int            qFormatNorm;
    Int            qPredictedSamples;
    Bool           leaveGetLoop;
    MC_Info       *pMC_Info;
    FrameInfo     *pFrameInfo;
    tDec_Int_File *pVars;
    tDec_Int_Chan *pChVars[Chans];

    per_chan_share_w_fxpCoef *pChLeftShare;
    per_chan_share_w_fxpCoef *pChRightShare;

    Int            status = MP4AUDEC_SUCCESS;

    Bool empty_frame;

#ifdef AAC_PLUS

    SBRDECODER_DATA *sbrDecoderData;
    SBR_DEC         *sbrDec;
    SBRBITSTREAM    *sbrBitStream;
#endif
	Int32 i;

    pVars = (tDec_Int_File *)pMem;

    pMC_Info = &pVars->mc_info;

    pChVars[LEFT]  = &pVars->perChan[LEFT];
    pChVars[RIGHT] = &pVars->perChan[RIGHT];

    pChLeftShare = pChVars[LEFT]->pShareWfxpCoef;
    pChRightShare = pChVars[RIGHT]->pShareWfxpCoef;

#ifdef AAC_PLUS

    sbrDecoderData = (SBRDECODER_DATA *) & pVars->sbrDecoderData;
    sbrDec         = (SBR_DEC *) & pVars->sbrDec;
    sbrBitStream   = (SBRBITSTREAM *) & pVars->sbrBitStr;

#ifdef PARAMETRICSTEREO
    sbrDecoderData->hParametricStereoDec = (HANDLE_PS_DEC) & pVars->sbrDecoderData.ParametricStereoDec;
#endif

#endif

    pVars->inputStream.pBuffer = pExt->pInputBuffer;

    pVars->inputStream.inputBufferCurrentLength = (UInt)pExt->inputBufferCurrentLength;

    pVars->inputStream.availableBits =
        (UInt)(pExt->inputBufferCurrentLength << INBUF_ARRAY_INDEX_SHIFT);

    initialUsedBits =
        (UInt)((pExt->inputBufferUsedLength << INBUF_ARRAY_INDEX_SHIFT) +
               pExt->remainderBits);

    pVars->inputStream.usedBits = initialUsedBits;

    if (initialUsedBits > pVars->inputStream.availableBits)
    {
        status = MP4AUDEC_INVALID_FRAME;
    }
    else if (pVars->bno == 0)
    {

        status =
            get_adif_header(
                pVars,
                &(pVars->scratch.scratch_prog_config));

        byte_align(&pVars->inputStream);

        if (status == SUCCESS)
        {
            pVars->prog_config.file_is_adts = FALSE;
        }
        else
        {
            pVars->prog_config.file_is_adts = TRUE;
        }
    }
    else if ((pVars->bno == 1) && (pVars->prog_config.file_is_adts == FALSE))
    {

        id_syn_ele = (Int)getbits(LEN_SE_ID, &pVars->inputStream) ;

        if (id_syn_ele == ID_END)
        {

            byte_align(&pVars->inputStream);

            pExt->inputBufferUsedLength =
                pVars->inputStream.usedBits >> INBUF_ARRAY_INDEX_SHIFT;

            pExt->remainderBits = pVars->inputStream.usedBits & INBUF_BIT_MODULO_MASK;

            pVars->bno++;

            return(status);
        }
        else
        {

            pVars->inputStream.usedBits -= LEN_SE_ID;
        }

    }

    if (pVars->prog_config.file_is_adts == TRUE)
    {

        if (pVars->prog_config.headerless_frames)
        {
            pVars->prog_config.headerless_frames--;
        }
        else
        {
            status =  get_adts_header(pVars,
                                      &(pVars->syncword),
                                      &(pVars->invoke),
                                      3);

            if (status != SUCCESS)
            {
                status = MP4AUDEC_LOST_FRAME_SYNC;
            }
        }
    }
    else
    {
        byte_align(&pVars->inputStream);
    }

#ifdef AAC_PLUS
    sbrBitStream->NrElements = 0;
    sbrBitStream->NrElementsCore = 0;

#endif

    leaveGetLoop = FALSE;
    empty_frame  = TRUE;

    while ((leaveGetLoop == FALSE) && (status == SUCCESS))
    {

        id_syn_ele = (Int)get9_n_lessbits(LEN_SE_ID, &pVars->inputStream);

        if (pVars->inputStream.usedBits > pVars->inputStream.availableBits)
        {
            status = MP4AUDEC_INCOMPLETE_FRAME;
            id_syn_ele = ID_END;
        }

        switch (id_syn_ele)
        {
            case ID_END:
                leaveGetLoop = TRUE;
                break;

            case ID_SCE:
            case ID_CPE:
                empty_frame = FALSE;
                status =
                    huffdecode(
                        id_syn_ele,
                        &(pVars->inputStream),
                        pVars,
                        pChVars);

#ifdef AAC_PLUS
                if (id_syn_ele == ID_SCE)
                {
                    sbrBitStream->sbrElement[sbrBitStream->NrElements].ElementID = SBR_ID_SCE;
                }
                else if (id_syn_ele == ID_CPE)
                {
                    sbrBitStream->sbrElement[sbrBitStream->NrElements].ElementID = SBR_ID_CPE;
                }
                sbrBitStream->NrElementsCore++;

#endif

                break;

            case ID_PCE:

                if (pVars->bno <= 1)
                {
                    status = get_prog_config(pVars,
                                             &(pVars->scratch.scratch_prog_config));
                }
                else
                {
                    status = MP4AUDEC_INVALID_FRAME;
                }
                break;

            case ID_FIL:
#ifdef AAC_PLUS
                get_sbr_bitstream(sbrBitStream, &pVars->inputStream);

#else
                getfill(&pVars->inputStream);
#endif

                break;

            case ID_DSE:
                get_dse(pVars->share.data_stream_bytes,
                        &pVars->inputStream);
                break;

            default:
                status = -1;
                break;

        }

    }

    byte_align(&pVars->inputStream);

#ifdef AAC_PLUS
    if (pVars->bno <= 1)
    {
        if ((pVars->mc_info.ExtendedAudioObjectType == MP4AUDIO_AAC_LC) &&
                (!sbrBitStream->NrElements))
        {
            PVMP4AudioDecoderDisableAacPlus(pExt, pMem);
        }
    }
#endif

    if (empty_frame == TRUE)
    {
        pExt->inputBufferUsedLength =
            pVars->inputStream.usedBits >> INBUF_ARRAY_INDEX_SHIFT;

        pExt->remainderBits = pVars->inputStream.usedBits & INBUF_BIT_MODULO_MASK;

        pVars->bno++;

        return(status);

    }

#ifdef AAC_PLUS

    if (sbrBitStream->NrElements)
    {

        if (sbrBitStream->NrElements != sbrBitStream->NrElementsCore)
        {
            status = MP4AUDEC_INVALID_FRAME;
        }

        if (pExt->aacPlusEnabled == FALSE)
        {
            sbrBitStream->NrElements = 0;
        }
    }
    else
    {

        if (pMC_Info->sbrPresentFlag || pMC_Info->psPresentFlag)
        {
            status = MP4AUDEC_INVALID_FRAME;
        }
    }
#endif

    frameLength = pVars->frameLength;

    if (status == SUCCESS)
    {

        pFrameInfo = pVars->winmap[pChVars[LEFT]->wnd];

        pns_left(
            pFrameInfo,
            pChLeftShare->group,
            pChLeftShare->cb_map,
            pChLeftShare->factors,
            pChLeftShare->lt_status.sfb_prediction_used,
            pChLeftShare->lt_status.ltp_data_present,
            pChVars[LEFT]->fxpCoef,
            pChLeftShare->qFormat,
            &(pVars->pns_cur_noise_state));

        if (pVars->hasmask > 0)
        {
            apply_ms_synt(
                pFrameInfo,
                pChLeftShare->group,
                pVars->mask,
                pChLeftShare->cb_map,
                pChVars[LEFT]->fxpCoef,
                pChVars[RIGHT]->fxpCoef,
                pChLeftShare->qFormat,
                pChRightShare->qFormat);
        }

        for (ch = 0; (ch < pMC_Info->nch); ch++)
        {
            pFrameInfo = pVars->winmap[pChVars[ch]->wnd];

            if (ch > 0)
            {
                pns_intensity_right(
                    pVars->hasmask,
                    pFrameInfo,
                    pChRightShare->group,
                    pVars->mask,
                    pChRightShare->cb_map,
                    pChLeftShare->factors,
                    pChRightShare->factors,
                    pChRightShare->lt_status.sfb_prediction_used,
                    pChRightShare->lt_status.ltp_data_present,
                    pChVars[LEFT]->fxpCoef,
                    pChVars[RIGHT]->fxpCoef,
                    pChLeftShare->qFormat,
                    pChRightShare->qFormat,
                    &(pVars->pns_cur_noise_state));
            }

            if (pChVars[ch]->pShareWfxpCoef->lt_status.ltp_data_present != FALSE)
            {

                qPredictedSamples = long_term_prediction(
                                        pChVars[ch]->wnd,
                                        pChVars[ch]->pShareWfxpCoef->lt_status.
                                        weight_index,
                                        pChVars[ch]->pShareWfxpCoef->lt_status.
                                        delay,
                                        pChVars[ch]->ltp_buffer,
                                        pVars->ltp_buffer_state,
                                        pChVars[ch]->time_quant,
                                        pVars->share.predictedSamples,
                                        frameLength);

                trans4m_time_2_freq_fxp(
                    pVars->share.predictedSamples,
                    pChVars[ch]->wnd,
                    pChVars[ch]->wnd_shape_prev_bk,
                    pChVars[ch]->wnd_shape_this_bk,
                    &qPredictedSamples,
                    pVars->scratch.fft);

                apply_tns(
                    pVars->share.predictedSamples,
                    pChVars[ch]->pShareWfxpCoef->qFormat,
                    pFrameInfo,
                    &(pChVars[ch]->pShareWfxpCoef->tns),
                    TRUE,
                    pVars->scratch.tns_inv_filter);

                long_term_synthesis(
                    pChVars[ch]->wnd,
                    pChVars[ch]->pShareWfxpCoef->max_sfb,
                    pFrameInfo->win_sfb_top[0],
                    pChVars[ch]->pShareWfxpCoef->lt_status.win_prediction_used,
                    pChVars[ch]->pShareWfxpCoef->lt_status.sfb_prediction_used,
                    pChVars[ch]->fxpCoef,
                    pChVars[ch]->pShareWfxpCoef->qFormat,
                    pVars->share.predictedSamples,
                    qPredictedSamples,
                    pFrameInfo->coef_per_win[0],
                    NUM_SHORT_WINDOWS,
                    NUM_RECONSTRUCTED_SFB);

            }

        }

        for (ch = 0; (ch < pMC_Info->nch); ch++)
        {

            pFrameInfo = pVars->winmap[pChVars[ch]->wnd];

            apply_tns(
                pChVars[ch]->fxpCoef,
                pChVars[ch]->pShareWfxpCoef->qFormat,
                pFrameInfo,
                &(pChVars[ch]->pShareWfxpCoef->tns),
                FALSE,
                pVars->scratch.tns_inv_filter);

            qFormatNorm =
                q_normalize(
                    pChVars[ch]->pShareWfxpCoef->qFormat,
                    pFrameInfo,
                    pChVars[ch]->abs_max_per_window,
                    pChVars[ch]->fxpCoef);

#ifdef AAC_PLUS
            if (sbrBitStream->NrElements == 0 && pMC_Info->upsamplingFactor == 1)
            {
                trans4m_freq_2_time_fxp_2(
                    pChVars[ch]->fxpCoef,
                    pChVars[ch]->time_quant,
                    pChVars[ch]->wnd,
                    pChVars[ch]->wnd_shape_prev_bk,
                    pChVars[ch]->wnd_shape_this_bk,
                    qFormatNorm,
                    pChVars[ch]->abs_max_per_window,
                    pVars->scratch.fft,
                    &pExt->pOutputBuffer[ch]);

                if (pVars->mc_info.audioObjectType == MP4AUDIO_LTP)
                {
                    Int16 * pt = &pExt->pOutputBuffer[ch];
                    Int16 * ptr = &(pChVars[ch]->ltp_buffer[pVars->ltp_buffer_state]);
                    Int16  x, y;
                    for (i = HALF_LONG_WINDOW; i != 0; i--)
                    {
                        x = *pt;
                        pt += 2;
                        y = *pt;
                        pt += 2;
                        *(ptr++) =  x;
                        *(ptr++) =  y;
                    }
                }
            }
            else
            {
                trans4m_freq_2_time_fxp_1(
                    pChVars[ch]->fxpCoef,
                    pChVars[ch]->time_quant,
                    &(pChVars[ch]->ltp_buffer[pVars->ltp_buffer_state + 288]),
                    pChVars[ch]->wnd,
                    pChVars[ch]->wnd_shape_prev_bk,
                    pChVars[ch]->wnd_shape_this_bk,
                    qFormatNorm,
                    pChVars[ch]->abs_max_per_window,
                    pVars->scratch.fft);

            }
#else

            trans4m_freq_2_time_fxp_2(
                pChVars[ch]->fxpCoef,
                pChVars[ch]->time_quant,
                pChVars[ch]->wnd,
                pChVars[ch]->wnd_shape_prev_bk,
                pChVars[ch]->wnd_shape_this_bk,
                qFormatNorm,
                pChVars[ch]->abs_max_per_window,
                pVars->scratch.fft,
                &pExt->pOutputBuffer[ch]);

            if (pVars->mc_info.audioObjectType == MP4AUDIO_LTP)
            {
                Int16 * pt = &pExt->pOutputBuffer[ch];
                Int16 * ptr = &(pChVars[ch]->ltp_buffer[pVars->ltp_buffer_state]);
                Int16  x, y;
                for (i = HALF_LONG_WINDOW; i != 0; i--)
                {
                    x = *pt;
                    pt += 2;
                    y = *pt;
                    pt += 2;
                    *(ptr++) =  x;
                    *(ptr++) =  y;
                }

            }

#endif

            pChVars[ch]->wnd_shape_prev_bk = pChVars[ch]->wnd_shape_this_bk;

        }

#ifdef AAC_PLUS

        if (sbrBitStream->NrElements || pMC_Info->upsamplingFactor == 2)
        {

            if (pVars->bno <= 1)
            {
                if (sbrDec->outSampleRate == 0)
                {
                    sbr_open(samp_rate_info[pVars->mc_info.sampling_rate_idx].samp_rate,
                             sbrDec,
                             sbrDecoderData,
                             pVars->mc_info.bDownSampledSbr);
                }

            }
            pMC_Info->upsamplingFactor =
                sbrDecoderData->SbrChannel[0].frameData.sbr_header.sampleRateMode;

            {
                Int16 *pt_left  =  &(pChVars[LEFT ]->ltp_buffer[pVars->ltp_buffer_state]);
                Int16 *pt_right =  &(pChVars[RIGHT]->ltp_buffer[pVars->ltp_buffer_state]);

                if (sbr_applied(sbrDecoderData,
                                sbrBitStream,
                                pt_left,
                                pt_right,
                                pExt->pOutputBuffer,
                                sbrDec,
                                pVars,
                                pMC_Info->nch) != SBRDEC_OK)
                {
                    status = MP4AUDEC_INVALID_FRAME;
                }
            }

        }
#endif

        if (pExt->desiredChannels == 2)
        {

#if defined(AAC_PLUS)
#if defined(PARAMETRICSTEREO)&&defined(HQ_SBR)
            if (pMC_Info->nch != 2 && pMC_Info->psPresentFlag != 1)
#else
            if (pMC_Info->nch != 2)
#endif
#else
            if (pMC_Info->nch != 2)
#endif
            {

                Int16 * pt  = &pExt->pOutputBuffer[0];
                Int16 * pt2 = &pExt->pOutputBuffer[1];
                Int i;
                if (pMC_Info->upsamplingFactor == 2)
                {
                    for (i = 0; i < 1024; i++)
                    {
                        *pt2 = *pt;
                        pt += 2;
                        pt2 += 2;
                    }
                    pt  = &pExt->pOutputBuffer_plus[0];
                    pt2 = &pExt->pOutputBuffer_plus[1];

                    for (i = 0; i < 1024; i++)
                    {
                        *pt2 = *pt;
                        pt += 2;
                        pt2 += 2;
                    }
                }
                else
                {
                    for (i = 0; i < 1024; i++)
                    {
                        *pt2 = *pt;
                        pt += 2;
                        pt2 += 2;
                    }
                }

            }

#if defined(AAC_PLUS)
#if defined(PARAMETRICSTEREO)&&defined(HQ_SBR)

            else if (pMC_Info->psPresentFlag == 1)
            {
                Int32 frameSize = 0;
                if (pExt->aacPlusEnabled == FALSE)
                {

                    frameSize = 1024;
                }
                else if (sbrDecoderData->SbrChannel[0].syncState != SBR_ACTIVE)
                {

                    frameSize = 2048;
                }

                Int16 * pt  = &pExt->pOutputBuffer[0];
                Int16 * pt2 = &pExt->pOutputBuffer[1];
                Int i;
                for (i = 0; i < frameSize; i++)
                {
                    *pt2 = *pt;
                    pt += 2;
                    pt2 += 2;
                }
            }
#endif
#endif

        }
        else
        {

#if defined(AAC_PLUS)
#if defined(PARAMETRICSTEREO)&&defined(HQ_SBR)
            if (pMC_Info->nch != 2 && pMC_Info->psPresentFlag != 1)
#else
            if (pMC_Info->nch != 2)
#endif
#else
            if (pMC_Info->nch != 2)
#endif
            {

                Int16 * pt  = &pExt->pOutputBuffer[0];
                Int16 * pt2 = &pExt->pOutputBuffer[0];
                Int i;

                if (pMC_Info->upsamplingFactor == 2)
                {
                    for (i = 0; i < 1024; i++)
                    {
                        *pt2++ = *pt;
                        pt += 2;
                    }

                    pt  = &pExt->pOutputBuffer_plus[0];
                    pt2 = &pExt->pOutputBuffer_plus[0];

                    for (i = 0; i < 1024; i++)
                    {
                        *pt2++ = *pt;
                        pt += 2;
                    }
                }
                else
                {
                    for (i = 0; i < 1024; i++)
                    {
                        *pt2++ = *pt;
                        pt += 2;
                    }
                }

            }

        }

#ifdef AAC_PLUS
        if (sbrBitStream->NrElements == 0 && pMC_Info->upsamplingFactor == 1)
        {
            pVars->ltp_buffer_state ^= frameLength;
        }
        else
        {
            pVars->ltp_buffer_state ^= (frameLength + 288);
        }
#else
        pVars->ltp_buffer_state ^= frameLength;
#endif

        if (pVars->bno <= 1)
        {

            pVars->ltp_buffer_state = 0;
            pExt->samplingRate =
                samp_rate_info[pVars->mc_info.sampling_rate_idx].samp_rate;

            pVars->mc_info.implicit_channeling = 0;

#ifdef AAC_PLUS

            if (pMC_Info->upsamplingFactor == 2)
            {
                pExt->samplingRate *= pMC_Info->upsamplingFactor;
                pExt->aacPlusUpsamplingFactor = pMC_Info->upsamplingFactor;
            }

#endif

            pExt->extendedAudioObjectType = pMC_Info->ExtendedAudioObjectType;
            pExt->audioObjectType = pMC_Info->audioObjectType;

            pExt->encodedChannels = pMC_Info->nch;
            pExt->frameLength = pVars->frameLength;
        }

        pVars->bno++;

        pExt->bitRate = (pExt->samplingRate *
                         (pVars->inputStream.usedBits - initialUsedBits)) >> 10;

        pExt->bitRate >>= (pMC_Info->upsamplingFactor - 1);

    }

    if (status != MP4AUDEC_SUCCESS)
    {

        if (pVars->prog_config.file_is_adts == TRUE)
        {
            status = MP4AUDEC_LOST_FRAME_SYNC;
            pVars->prog_config.headerless_frames = 0;
        }
        else
        {

            if (pVars->inputStream.usedBits > pVars->inputStream.availableBits)
            {

                pVars->inputStream.usedBits = pVars->inputStream.availableBits;

                status = MP4AUDEC_INCOMPLETE_FRAME;
            }
        }
    }

    pExt->inputBufferUsedLength =
        pVars->inputStream.usedBits >> INBUF_ARRAY_INDEX_SHIFT;

    pExt->remainderBits = (Int)(pVars->inputStream.usedBits & INBUF_BIT_MODULO_MASK);

    return (status);

}

