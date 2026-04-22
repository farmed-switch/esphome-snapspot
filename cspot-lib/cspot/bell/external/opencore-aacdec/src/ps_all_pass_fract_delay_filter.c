

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

void ps_all_pass_fract_delay_filter_type_I(UInt32 *delayBufIndex,
        Int32 sb_delay,
        const Int32 *ppFractDelayPhaseFactorSer,
        Int32 ***pppRealDelayRBufferSer,
        Int32 ***pppImagDelayRBufferSer,
        Int32 *rIn,
        Int32 *iIn)
{

    Int32 cmplx;
    Int16 rTmp0;
    Int32 rTmp;
    Int32 iTmp;
    Int32 *pt_rTmp;
    Int32 *pt_iTmp;

    Int32 tmp_r;
    Int32 tmp_i;

    pt_rTmp = &pppRealDelayRBufferSer[0][*(delayBufIndex)][sb_delay];
    pt_iTmp = &pppImagDelayRBufferSer[0][*(delayBufIndex++)][sb_delay];

    cmplx  = *(ppFractDelayPhaseFactorSer++);
    tmp_r = *pt_rTmp << 1;
    tmp_i = *pt_iTmp << 1;

    rTmp = cmplx_mul32_by_16(tmp_r, -tmp_i,  cmplx);
    rTmp0  = Qfmt15(0.65143905753106f);
    iTmp = cmplx_mul32_by_16(tmp_i,  tmp_r,  cmplx);

    iTmp     =  fxp_mac32_by_16(-*iIn << 1, rTmp0, iTmp);
    *pt_iTmp =  fxp_mac32_by_16(iTmp << 1, rTmp0, *iIn);
    *iIn = iTmp;

    rTmp     =  fxp_mac32_by_16(-*rIn << 1, rTmp0, rTmp);
    *pt_rTmp =  fxp_mac32_by_16(rTmp << 1, rTmp0, *rIn);

    *rIn = rTmp;

    pt_rTmp = &pppRealDelayRBufferSer[1][*(delayBufIndex)][sb_delay];
    pt_iTmp = &pppImagDelayRBufferSer[1][*(delayBufIndex++)][sb_delay];

    cmplx  = *(ppFractDelayPhaseFactorSer++);
    tmp_r = *pt_rTmp << 1;
    tmp_i = *pt_iTmp << 1;

    rTmp = cmplx_mul32_by_16(tmp_r, -tmp_i,  cmplx);
    rTmp0  = Qfmt15(0.56471812200776f);
    iTmp = cmplx_mul32_by_16(tmp_i,  tmp_r,  cmplx);

    iTmp     =  fxp_mac32_by_16(-*iIn << 1, rTmp0, iTmp);
    *pt_iTmp =  fxp_mac32_by_16(iTmp << 1, rTmp0, *iIn);
    *iIn = iTmp;

    rTmp     =  fxp_mac32_by_16(-*rIn << 1, rTmp0, rTmp);
    *pt_rTmp =  fxp_mac32_by_16(rTmp << 1, rTmp0, *rIn);
    *rIn = rTmp;

    pt_rTmp = &pppRealDelayRBufferSer[2][*(delayBufIndex)][sb_delay];
    pt_iTmp = &pppImagDelayRBufferSer[2][*(delayBufIndex)][sb_delay];

    cmplx  = *(ppFractDelayPhaseFactorSer);
    tmp_r = *pt_rTmp << 1;
    tmp_i = *pt_iTmp << 1;

    rTmp = cmplx_mul32_by_16(tmp_r, -tmp_i,  cmplx);
    rTmp0  = Qfmt15(0.97908331911390f);
    iTmp = cmplx_mul32_by_16(tmp_i,  tmp_r,  cmplx);

    iTmp     =  fxp_mac32_by_16(-*iIn, rTmp0, iTmp);
    *pt_iTmp =  fxp_mac32_by_16(iTmp, rTmp0, *iIn);
    *iIn = iTmp << 2;

    rTmp     =  fxp_mac32_by_16(-*rIn, rTmp0, rTmp);
    *pt_rTmp =  fxp_mac32_by_16(rTmp, rTmp0, *rIn);
    *rIn = rTmp << 2;
}

void ps_all_pass_fract_delay_filter_type_II(UInt32 *delayBufIndex,
        Int32 sb_delay,
        const Int32 *ppFractDelayPhaseFactorSer,
        Int32 ***pppRealDelayRBufferSer,
        Int32 ***pppImagDelayRBufferSer,
        Int32 *rIn,
        Int32 *iIn,
        Int32 decayScaleFactor)
{

    Int32 cmplx;
    Int16 rTmp0;
    Int32 rTmp;
    Int32 iTmp;
    Int32 *pt_rTmp;
    Int32 *pt_iTmp;
    const Int16 *pt_delay;

    Int32 tmp_r;
    Int32 tmp_i;

    pt_rTmp = &pppRealDelayRBufferSer[0][*(delayBufIndex)][sb_delay];
    pt_iTmp = &pppImagDelayRBufferSer[0][*(delayBufIndex++)][sb_delay];

    cmplx  = *(ppFractDelayPhaseFactorSer++);
    pt_delay = aRevLinkDecaySerCoeff[decayScaleFactor];
    tmp_r = *pt_rTmp << 1;
    tmp_i = *pt_iTmp << 1;

    rTmp = cmplx_mul32_by_16(tmp_r, -tmp_i,  cmplx);
    rTmp0  = *(pt_delay++);
    iTmp = cmplx_mul32_by_16(tmp_i,  tmp_r,  cmplx);

    iTmp     =  fxp_mac32_by_16(-*iIn << 1, rTmp0, iTmp);
    *pt_iTmp =  fxp_mac32_by_16(iTmp << 1, rTmp0, *iIn);
    *iIn = iTmp;

    rTmp     =  fxp_mac32_by_16(-*rIn << 1, rTmp0, rTmp);
    *pt_rTmp =  fxp_mac32_by_16(rTmp << 1, rTmp0, *rIn);
    *rIn = rTmp;

    pt_rTmp = &pppRealDelayRBufferSer[1][*(delayBufIndex)][sb_delay];
    pt_iTmp = &pppImagDelayRBufferSer[1][*(delayBufIndex++)][sb_delay];

    cmplx  = *(ppFractDelayPhaseFactorSer++);
    tmp_r = *pt_rTmp << 1;
    tmp_i = *pt_iTmp << 1;

    rTmp = cmplx_mul32_by_16(tmp_r, -tmp_i,  cmplx);
    rTmp0  = *(pt_delay++);
    iTmp = cmplx_mul32_by_16(tmp_i,  tmp_r,  cmplx);
    iTmp     =  fxp_mac32_by_16(-*iIn << 1, rTmp0, iTmp);
    *pt_iTmp =  fxp_mac32_by_16(iTmp << 1, rTmp0, *iIn);
    *iIn = iTmp;

    rTmp     =  fxp_mac32_by_16(-*rIn << 1, rTmp0, rTmp);
    *pt_rTmp =  fxp_mac32_by_16(rTmp << 1, rTmp0, *rIn);
    *rIn = rTmp;

    pt_rTmp = &pppRealDelayRBufferSer[2][*(delayBufIndex)][sb_delay];
    pt_iTmp = &pppImagDelayRBufferSer[2][*(delayBufIndex)][sb_delay];

    cmplx  = *(ppFractDelayPhaseFactorSer);
    tmp_r = *pt_rTmp << 1;
    tmp_i = *pt_iTmp << 1;

    rTmp = cmplx_mul32_by_16(tmp_r, -tmp_i,  cmplx);
    rTmp0  = *(pt_delay);
    iTmp = cmplx_mul32_by_16(tmp_i,  tmp_r,  cmplx);

    iTmp     =  fxp_mac32_by_16(-*iIn, rTmp0, iTmp);
    *pt_iTmp =  fxp_mac32_by_16(iTmp, rTmp0, *iIn);
    *iIn = iTmp << 2;

    rTmp     =  fxp_mac32_by_16(-*rIn, rTmp0, rTmp);
    *pt_rTmp =  fxp_mac32_by_16(rTmp, rTmp0, *rIn);
    *rIn = rTmp << 2;

}

#endif

#endif

