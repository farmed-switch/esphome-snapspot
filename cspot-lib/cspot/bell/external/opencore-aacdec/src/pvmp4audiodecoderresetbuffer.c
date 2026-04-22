

#include "pv_audio_type_defs.h"
#include "s_tdec_int_file.h"
#include "pvmp4audiodecoder_api.h"
#include "aac_mem_funcs.h"
#include "config.h"

#ifdef AAC_PLUS
#include    "s_sbr_frame_data.h"
#endif

#define LEFT  (0)
#define RIGHT (1)

OSCL_EXPORT_REF void PVMP4AudioDecoderResetBuffer(void  *pMem)
{

    tDec_Int_File *pVars;

#ifdef AAC_PLUS
    SBR_FRAME_DATA * hFrameData_1;
    SBR_FRAME_DATA * hFrameData_2;
#ifdef HQ_SBR
#ifdef PARAMETRICSTEREO
    SBRDECODER_DATA *sbrDecoderData;
#endif
#endif

#endif

    pVars = (tDec_Int_File *)pMem;

    pv_memset(pVars->perChan[LEFT].time_quant,
              0,
              LONG_WINDOW*sizeof(pVars->perChan[LEFT].time_quant[0]));

    pv_memset(pVars->perChan[RIGHT].time_quant,
              0,
              LONG_WINDOW*sizeof(pVars->perChan[RIGHT].time_quant[0]));

#ifdef AAC_PLUS

    if (!pVars->sbrDecoderData.setStreamType)
    {
        if (pVars->aacPlusEnabled == TRUE)
        {

            hFrameData_1   = (SBR_FRAME_DATA *) & pVars->sbrDecoderData.SbrChannel[LEFT].frameData;
            hFrameData_2   = (SBR_FRAME_DATA *) & pVars->sbrDecoderData.SbrChannel[RIGHT].frameData;
#ifdef HQ_SBR
#ifdef PARAMETRICSTEREO
            sbrDecoderData = (SBRDECODER_DATA *) & pVars->sbrDecoderData;
            sbrDecoderData->hParametricStereoDec = (HANDLE_PS_DEC) & pVars->sbrDecoderData.ParametricStereoDec;
#endif
#endif

            pv_memset(&pVars->perChan[LEFT].ltp_buffer[0],
                      0,
                      288*sizeof(pVars->perChan[LEFT].ltp_buffer[0]));
            pv_memset(&pVars->perChan[LEFT].ltp_buffer[1024 + 288],
                      0,
                      288*sizeof(pVars->perChan[LEFT].ltp_buffer[0]));
            pv_memset(hFrameData_1->V,
                      0,
                      1152*sizeof(hFrameData_1->V[0]));
            pv_memset(hFrameData_1->prevNoiseLevel_man,
                      0,
                      MAX_NUM_NOISE_VALUES*sizeof(hFrameData_1->prevNoiseLevel_man[0]));

            pv_memset(&pVars->perChan[RIGHT].ltp_buffer[0],
                      0,
                      288*sizeof(pVars->perChan[RIGHT].ltp_buffer[0]));
            pv_memset(&pVars->perChan[RIGHT].ltp_buffer[1024 + 288],
                      0,
                      288*sizeof(pVars->perChan[RIGHT].ltp_buffer[0]));
            pv_memset(hFrameData_2->V,
                      0,
                      1152*sizeof(hFrameData_2->V[0]));

            pv_memset(hFrameData_2->prevNoiseLevel_man,
                      0,
                      MAX_NUM_NOISE_VALUES*sizeof(hFrameData_2->prevNoiseLevel_man[0]));

            int i;
            for (i = 0; i < 8; i++)
            {
                pv_memset((void *)&hFrameData_1->codecQmfBufferReal[i],
                          0,
                          sizeof(**hFrameData_1->codecQmfBufferReal) << 5);
            }

            pv_memset((void *)hFrameData_1->BwVectorOld,
                      0,
                      sizeof(*hFrameData_1->BwVectorOld)*MAX_NUM_PATCHES);

#ifdef HQ_SBR

            for (i = 0; i < 5; i++)
            {
                pv_memset((void *)&hFrameData_1->fBuffer_man[i],
                          0,
                          sizeof(**hFrameData_1->fBuffer_man)*64);
                pv_memset((void *)&hFrameData_1->fBufferN_man[i],
                          0,
                          sizeof(**hFrameData_1->fBufferN_man)*64);
            }
#endif

            pv_memset((void *)hFrameData_1->HistsbrQmfBufferReal,
                      0,
                      sizeof(*hFrameData_1->HistsbrQmfBufferReal)*6*SBR_NUM_BANDS);

#ifdef HQ_SBR
            pv_memset((void *)hFrameData_1->HistsbrQmfBufferImag,
                      0,
                      sizeof(*hFrameData_1->HistsbrQmfBufferImag)*6*SBR_NUM_BANDS);
#endif

            if (pVars->sbrDec.LC_aacP_DecoderFlag == 1)
            {

                for (i = 0; i < 8; i++)
                {
                    pv_memset((void *)&hFrameData_2->codecQmfBufferReal[i],
                              0,
                              sizeof(**hFrameData_1->codecQmfBufferReal) << 5);
                }

                pv_memset((void *)hFrameData_2->HistsbrQmfBufferReal,
                          0,
                          sizeof(*hFrameData_2->HistsbrQmfBufferReal)*6*SBR_NUM_BANDS);

                pv_memset((void *)hFrameData_2->BwVectorOld,
                          0,
                          sizeof(*hFrameData_2->BwVectorOld)*MAX_NUM_PATCHES);

#ifdef HQ_SBR

                for (i = 0; i < 5; i++)
                {
                    pv_memset((void *)&hFrameData_2->fBuffer_man[i],
                              0,
                              sizeof(**hFrameData_2->fBuffer_man)*64);
                    pv_memset((void *)&hFrameData_2->fBufferN_man[i],
                              0,
                              sizeof(**hFrameData_2->fBufferN_man)*64);
                }
#endif

            }

#ifdef HQ_SBR
#ifdef PARAMETRICSTEREO
            else if (pVars->mc_info.psPresentFlag == 1)
            {
                for (i = 0; i < 3; i++)
                {
                    pv_memset(sbrDecoderData->hParametricStereoDec->hHybrid->mQmfBufferReal[i],
                              0,
                              HYBRID_FILTER_LENGTH_m_1*sizeof(*sbrDecoderData->hParametricStereoDec->hHybrid->mQmfBufferReal));
                    pv_memset(sbrDecoderData->hParametricStereoDec->hHybrid->mQmfBufferImag[i],
                              0,
                              HYBRID_FILTER_LENGTH_m_1*sizeof(*sbrDecoderData->hParametricStereoDec->hHybrid->mQmfBufferImag));
                }
            }
#endif
#endif

            pVars->sbrDecoderData.SbrChannel[LEFT].syncState = UPSAMPLING;
            pVars->sbrDecoderData.SbrChannel[RIGHT].syncState = UPSAMPLING;

        }
    }
#endif

    pVars->bno = 1;

    return ;

}

