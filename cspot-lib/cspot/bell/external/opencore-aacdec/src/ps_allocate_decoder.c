

#include    "config.h"

#ifdef AAC_PLUS

#ifdef HQ_SBR

#ifdef PARAMETRICSTEREO

#include    "s_sbr_channel.h"
#include    "aac_mem_funcs.h"
#include    "ps_hybrid_filter_bank_allocation.h"
#include    "s_ps_dec.h"
#include    "ps_all_pass_filter_coeff.h"
#include    "ps_allocate_decoder.h"

#define R_SHIFT     30
#define Q30_fmt(x)   (Int32)(x*((Int32)1<<R_SHIFT) + (x>=0?0.5F:-0.5F))

const Int32  aRevLinkDelaySer[] = {3,  4,  5};

Int32 ps_allocate_decoder(SBRDECODER_DATA *self,
                          UInt32  noSubSamples)
{
    Int32 i, j;
    Int32 status;

    Int32 *ptr1;
    Int32 *ptr2;
    Int32 *ptr3;
    Int32 *ptr4;
    Int32 *ptr5;
    Int32 *ptr6;
    Int32 *ptr7;

    const Int32 pHybridResolution[] = { HYBRID_8_CPLX,
                                        HYBRID_2_REAL,
                                        HYBRID_2_REAL
                                      };

    STRUCT_PS_DEC *h_ps_dec = self->hParametricStereoDec;

    h_ps_dec->noSubSamples = noSubSamples;

    h_ps_dec->invNoSubSamples = Q30_fmt(1.0f) / noSubSamples;

    ptr1 = (Int32 *)(self->SbrChannel[1].frameData.codecQmfBufferReal[0]);

    ptr2 = (&ptr1[658]);

    ptr3 = (&ptr1[1162]);

    ptr4 = (&ptr1[1426]);

    ptr5 = (&ptr1[1490]);

    ptr6 = (&ptr1[1618]);

    ptr7 = (&ptr1[1810]);

    h_ps_dec->aPeakDecayFast =  ptr1;
    ptr1 += NO_BINS;

    h_ps_dec->aPrevNrg =  ptr1;
    ptr1 += NO_BINS;

    h_ps_dec->aPrevPeakDiff = ptr1;
    ptr1 += NO_BINS;

    status = ps_hybrid_filter_bank_allocation(&h_ps_dec->hHybrid,
             NO_QMF_CHANNELS_IN_HYBRID,
             pHybridResolution,
             &ptr1);
    h_ps_dec->mHybridRealLeft = ptr1;
    ptr1 += SUBQMF_GROUPS;

    h_ps_dec->mHybridImagLeft = ptr1;
    ptr1 += SUBQMF_GROUPS;

    h_ps_dec->mHybridRealRight = ptr1;
    ptr1 += SUBQMF_GROUPS;

    h_ps_dec->mHybridImagRight = ptr1;
    ptr1 += SUBQMF_GROUPS;

    h_ps_dec->delayBufIndex   = 0;

    for (i = 0 ; i < NO_DELAY_CHANNELS ; i++)
    {
        if (i < SHORT_DELAY_START)
        {
            h_ps_dec->aNoSampleDelay[i] = LONG_DELAY;
        }
        else
        {
            h_ps_dec->aNoSampleDelay[i] = SHORT_DELAY;
        }
    }

    h_ps_dec->aaRealDelayBufferQmf = (Int32 **)ptr6;
    ptr6 += NO_QMF_ICC_CHANNELS * sizeof(Int32 *) / sizeof(Int32);

    h_ps_dec->aaImagDelayBufferQmf = (Int32 **)ptr7;
    ptr7 += NO_QMF_ICC_CHANNELS * sizeof(Int32 *) / sizeof(Int32);

    h_ps_dec->aaRealDelayBufferSubQmf = (Int32 **)ptr1;
    ptr1 += SUBQMF_GROUPS * sizeof(Int32 *) / sizeof(Int32);

    h_ps_dec->aaImagDelayBufferSubQmf = (Int32 **)ptr1;
    ptr1 += SUBQMF_GROUPS * sizeof(Int32 *) / sizeof(Int32);

    for (i = 0; i < NO_QMF_ICC_CHANNELS; i++)
    {
        int delay;

        if (i < NO_QMF_ALLPASS_CHANNELS)
        {
            delay = 2;
            h_ps_dec->aaRealDelayBufferQmf[i] = (Int32 *)ptr4;
            ptr4 += delay;

            h_ps_dec->aaImagDelayBufferQmf[i] = (Int32 *)ptr5;
            ptr5 += delay;
        }
        else
        {

            if (i >= (NO_QMF_ALLPASS_CHANNELS + SHORT_DELAY_START))
            {
                delay = SHORT_DELAY;
            }
            else
            {
                delay = LONG_DELAY;
            }

            h_ps_dec->aaRealDelayBufferQmf[i] = (Int32 *)ptr1;
            ptr1 += delay;

            h_ps_dec->aaImagDelayBufferQmf[i] = (Int32 *)ptr1;
            ptr1 += delay;
        }
    }

    for (i = 0; i < SUBQMF_GROUPS; i++)
    {
        h_ps_dec->aaRealDelayBufferSubQmf[i] = (Int32 *)ptr1;
        ptr1 += DELAY_ALLPASS;

        h_ps_dec->aaImagDelayBufferSubQmf[i] = (Int32 *)ptr1;
        ptr1 += DELAY_ALLPASS;

    }

    for (i = 0 ; i < NO_SERIAL_ALLPASS_LINKS ; i++)
    {

        h_ps_dec->aDelayRBufIndexSer[i] = 0;

        h_ps_dec->aaaRealDelayRBufferSerQmf[i] = (Int32 **)ptr2;
        ptr2 += aRevLinkDelaySer[i];

        h_ps_dec->aaaImagDelayRBufferSerQmf[i] = (Int32 **)ptr2;
        ptr2 += aRevLinkDelaySer[i];

        h_ps_dec->aaaRealDelayRBufferSerSubQmf[i] = (Int32 **)ptr3;
        ptr3 += aRevLinkDelaySer[i];

        h_ps_dec->aaaImagDelayRBufferSerSubQmf[i] = (Int32 **)ptr3;
        ptr3 += aRevLinkDelaySer[i];

        for (j = 0; j < aRevLinkDelaySer[i]; j++)
        {
            h_ps_dec->aaaRealDelayRBufferSerQmf[i][j] = ptr2;
            ptr2 += NO_QMF_ALLPASS_CHANNELS;

            h_ps_dec->aaaImagDelayRBufferSerQmf[i][j] = ptr2;
            ptr2 += NO_QMF_ALLPASS_CHANNELS;

            h_ps_dec->aaaRealDelayRBufferSerSubQmf[i][j] = ptr3;
            ptr3 += SUBQMF_GROUPS;

            h_ps_dec->aaaImagDelayRBufferSerSubQmf[i][j] = ptr3;
            ptr3 += SUBQMF_GROUPS;

        }
    }

    for (i = 0; i < NO_IID_GROUPS; i++)
    {
        h_ps_dec->h11Prev[i] = Q30_fmt(1.0f);
        h_ps_dec->h12Prev[i] = Q30_fmt(1.0f);
    }

    return status;
}
#endif

#endif

#endif

