

#include    "config.h"

#ifdef AAC_PLUS

#include    "init_sbr_dec.h"
#include    "aac_mem_funcs.h"
#include    "extractframeinfo.h"

Int32 init_sbr_dec(Int32 codecSampleRate,
                   Int   upsampleFac,
                   SBR_DEC *sbrDec,
                   SBR_FRAME_DATA *hFrameData)
{
    Int32 outFrameSize;
    Int32 coreCodecFrameSize = 1024;
#ifdef HQ_SBR
    Int32 i;
#endif

    sbrDec->sbStopCodec    =  upsampleFac << 5;
    sbrDec->prevLowSubband =  upsampleFac << 5;

    sbrDec->outSampleRate = 2 * codecSampleRate;
    outFrameSize = upsampleFac * coreCodecFrameSize;

    hFrameData->nSfb[LO] = 0;
    hFrameData->nSfb[HI] = 0;
    hFrameData->offset   = 0;

    hFrameData->nNfb = hFrameData->sbr_header.noNoiseBands;
    hFrameData->prevEnvIsShort = -1;

#ifdef HQ_SBR
    for (i = 0; i < 5; i++)
    {
        hFrameData->fBuf_man[i]  = hFrameData->fBuffer_man[i];
        hFrameData->fBufN_man[i] = hFrameData->fBufferN_man[i];
        hFrameData->fBuf_exp[i]  = hFrameData->fBuffer_exp[i];
        hFrameData->fBufN_exp[i] = hFrameData->fBufferN_exp[i];
    }
#endif

    pv_memset((void *)hFrameData->sbr_invf_mode_prev,
              0,
              MAX_NUM_NOISE_VALUES*sizeof(INVF_MODE));

    sbrDec->noCols = 32;

    sbrDec->bufWriteOffs = 6 + 2;
    sbrDec->bufReadOffs  = 2;
    sbrDec->qmfBufLen = sbrDec->noCols + sbrDec->bufWriteOffs;

    sbrDec->lowBandAddSamples = 288;

    sbrDec->startIndexCodecQmf = 0;

    sbrDec->lowSubband =  32;

    return outFrameSize;
}

#endif

