

#include    "config.h"

#ifdef AAC_PLUS

#ifdef PARAMETRICSTEREO
#include    "pv_audio_type_defs.h"
#include    "ps_decorrelate.h"
#include    "aac_mem_funcs.h"
#include    "ps_all_pass_filter_coeff.h"
#include    "ps_pwr_transient_detection.h"
#include    "ps_all_pass_fract_delay_filter.h"
#include    "fxp_mul32.h"

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

void ps_decorrelate(STRUCT_PS_DEC *h_ps_dec,
                    Int32 *rIntBufferLeft,
                    Int32 *iIntBufferLeft,
                    Int32 *rIntBufferRight,
                    Int32 *iIntBufferRight,
                    Int32 scratch_mem[])
{
    Int32 sb;
    Int32 maxsb;
    Int32 gr;
    Int32 sb_delay;
    Int32 bin;

    Int32 *aLeftReal;
    Int32 *aLeftImag;
    Int32 *aRightReal;
    Int32 *aRightImag;

    Int32 *aTransRatio = scratch_mem;

    Int32 ***pppRealDelayRBufferSer;
    Int32 ***pppImagDelayRBufferSer;

    Int32 **ppRealDelayBuffer;
    Int32 **ppImagDelayBuffer;

    const Int32(*ppFractDelayPhaseFactorSer)[3];

    ps_pwr_transient_detection(h_ps_dec,
                               rIntBufferLeft,
                               iIntBufferLeft,
                               aTransRatio);

    aLeftReal = h_ps_dec->mHybridRealLeft;
    aLeftImag = h_ps_dec->mHybridImagLeft;
    aRightReal = h_ps_dec->mHybridRealRight;
    aRightImag = h_ps_dec->mHybridImagRight;

    pppRealDelayRBufferSer = h_ps_dec->aaaRealDelayRBufferSerSubQmf;
    pppImagDelayRBufferSer = h_ps_dec->aaaImagDelayRBufferSerSubQmf;

    ppRealDelayBuffer = h_ps_dec->aaRealDelayBufferSubQmf;
    ppImagDelayBuffer = h_ps_dec->aaImagDelayBufferSubQmf;

    ppFractDelayPhaseFactorSer = aaFractDelayPhaseFactorSerSubQmf;

    for (gr = 0; gr < SUBQMF_GROUPS; gr++)
    {
        Int32 rIn;
        Int32 iIn;
        Int32 *pt_rTmp;
        Int32 *pt_iTmp;
        Int32 rTmp;
        Int32 cmplx;
        Int32 tmp1, tmp2;

        sb = groupBorders[gr];

        pt_rTmp = &ppRealDelayBuffer[sb][h_ps_dec->delayBufIndex];
        pt_iTmp = &ppImagDelayBuffer[sb][h_ps_dec->delayBufIndex];

        tmp1 = aLeftReal[sb];
        tmp2 = aLeftImag[sb];
        rIn = *pt_rTmp >> 1;
        iIn = *pt_iTmp >> 1;

        *pt_rTmp = tmp1;
        *pt_iTmp = tmp2;

        cmplx =  aFractDelayPhaseFactorSubQmf[sb];

        aRightReal[sb]  = cmplx_mul32_by_16(rIn, -iIn, cmplx);
        aRightImag[sb]  = cmplx_mul32_by_16(iIn,  rIn, cmplx);

        ps_all_pass_fract_delay_filter_type_I(h_ps_dec->aDelayRBufIndexSer,
                                              sb,
                                              ppFractDelayPhaseFactorSer[sb],
                                              pppRealDelayRBufferSer,
                                              pppImagDelayRBufferSer,
                                              &aRightReal[sb],
                                              &aRightImag[sb]);

        bin = bins2groupMap[gr];
        rTmp = aTransRatio[bin];

        if (rTmp != 0x7FFFFFFF)
        {
            aRightReal[sb] = fxp_mul32_Q31(rTmp, aRightReal[sb]) << 1;
            aRightImag[sb] = fxp_mul32_Q31(rTmp, aRightImag[sb]) << 1;
        }

    }

    aLeftReal = rIntBufferLeft;
    aLeftImag = iIntBufferLeft;
    aRightReal = rIntBufferRight;
    aRightImag = iIntBufferRight;

    pppRealDelayRBufferSer = h_ps_dec->aaaRealDelayRBufferSerQmf;
    pppImagDelayRBufferSer = h_ps_dec->aaaImagDelayRBufferSerQmf;

    ppRealDelayBuffer = h_ps_dec->aaRealDelayBufferQmf;
    ppImagDelayBuffer = h_ps_dec->aaImagDelayBufferQmf;

    ppFractDelayPhaseFactorSer = aaFractDelayPhaseFactorSerQmf;

    for (gr = SUBQMF_GROUPS; gr < NO_BINS; gr++)
    {

        maxsb = min(h_ps_dec->usb, groupBorders[gr+1]);

        for (sb = groupBorders[gr]; sb < maxsb; sb++)
        {

            Int32 rIn, iIn;
            Int32 *pt_rTmp, *pt_iTmp;
            Int32 cmplx;
            Int32 tmp1, tmp2;
            Int32 rTmp;

            sb_delay = sb - NO_QMF_CHANNELS_IN_HYBRID;

            pt_rTmp = &ppRealDelayBuffer[sb_delay][h_ps_dec->delayBufIndex];
            pt_iTmp = &ppImagDelayBuffer[sb_delay][h_ps_dec->delayBufIndex];

            rIn = *pt_rTmp >> 1;
            iIn = *pt_iTmp >> 1;

            tmp1 = aLeftReal[sb];
            tmp2 = aLeftImag[sb];
            *pt_rTmp = tmp1;
            *pt_iTmp = tmp2;

            cmplx =  aFractDelayPhaseFactor[sb_delay];
            aRightReal[sb] = cmplx_mul32_by_16(rIn, -iIn, cmplx);
            aRightImag[sb] = cmplx_mul32_by_16(iIn,  rIn, cmplx);

            ps_all_pass_fract_delay_filter_type_II(h_ps_dec->aDelayRBufIndexSer,
                                                   sb_delay,
                                                   ppFractDelayPhaseFactorSer[sb_delay],
                                                   pppRealDelayRBufferSer,
                                                   pppImagDelayRBufferSer,
                                                   &aRightReal[sb],
                                                   &aRightImag[sb],
                                                   sb);

            rTmp = aTransRatio[gr-2];
            if (rTmp != 0x7FFFFFFF)
            {
                aRightReal[sb] = fxp_mul32_Q31(rTmp, aRightReal[sb]) << 1;
                aRightImag[sb] = fxp_mul32_Q31(rTmp, aRightImag[sb]) << 1;
            }

        }

    }

    maxsb = min(h_ps_dec->usb, 35);

    {
        Int32 factor = aTransRatio[NO_BINS-2];

        for (sb = 23; sb < maxsb; sb++)
        {

            Int32  tmp, tmp2;
            Int32 *pt_rTmp, *pt_iTmp;

            sb_delay = sb - NO_QMF_CHANNELS_IN_HYBRID;

            Int32 k = sb - NO_ALLPASS_CHANNELS;

            pt_rTmp = &ppRealDelayBuffer[sb_delay][h_ps_dec->aDelayBufIndex[ k]];
            pt_iTmp = &ppImagDelayBuffer[sb_delay][h_ps_dec->aDelayBufIndex[ k]];

            if (++h_ps_dec->aDelayBufIndex[ k] >= LONG_DELAY)
            {
                h_ps_dec->aDelayBufIndex[ k] = 0;
            }

            tmp  = *pt_rTmp;
            tmp2 = *pt_iTmp;

            if (aTransRatio[NO_BINS-2] < 0x7FFFFFFF)
            {
                aRightReal[sb] = fxp_mul32_Q31(factor, tmp) << 1;
                aRightImag[sb] = fxp_mul32_Q31(factor, tmp2) << 1;
            }
            else
            {
                aRightReal[sb] = tmp;
                aRightImag[sb] = tmp2;
            }

            tmp  = aLeftReal[sb];
            tmp2 = aLeftImag[sb];
            *pt_rTmp = tmp;
            *pt_iTmp = tmp2;

        }
    }

    maxsb = min(h_ps_dec->usb, 64);

    {

        for (sb = 35; sb < maxsb; sb++)
        {

            Int32 *pt_rTmp, *pt_iTmp;

            sb_delay = sb - NO_QMF_CHANNELS_IN_HYBRID;

            pt_rTmp = &ppRealDelayBuffer[sb_delay][0];
            pt_iTmp = &ppImagDelayBuffer[sb_delay][0];

            aRightReal[sb] = *pt_rTmp;
            aRightImag[sb] = *pt_iTmp;

            if (aTransRatio[NO_BINS-1] < 0x7FFFFFFF)
            {
                aRightReal[sb] = fxp_mul32_Q31(aTransRatio[NO_BINS-1], aRightReal[sb]) << 1;
                aRightImag[sb] = fxp_mul32_Q31(aTransRatio[NO_BINS-1], aRightImag[sb]) << 1;
            }

            *pt_rTmp = aLeftReal[sb];
            *pt_iTmp = aLeftImag[sb];

        }
    }

    if (++h_ps_dec->delayBufIndex >= DELAY_ALLPASS)
    {
        h_ps_dec->delayBufIndex = 0;
    }

    if (++h_ps_dec->aDelayRBufIndexSer[0] >= 3)
    {
        h_ps_dec->aDelayRBufIndexSer[0] = 0;
    }
    if (++h_ps_dec->aDelayRBufIndexSer[1] >= 4)
    {
        h_ps_dec->aDelayRBufIndexSer[1] = 0;
    }
    if (++h_ps_dec->aDelayRBufIndexSer[2] >= 5)
    {
        h_ps_dec->aDelayRBufIndexSer[2] = 0;
    }

}
#endif

#endif

