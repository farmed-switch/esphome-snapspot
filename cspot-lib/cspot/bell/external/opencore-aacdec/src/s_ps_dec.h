

#ifndef S_PS_DEC_H
#define S_PS_DEC_H

#include "s_hybrid.h"
#include "s_patch.h"

struct PS_DEC
{

    Int psDetected;
    Int32 *R_ch_qmf_filter_history;
    Int32 invNoSubSamples;

    Int32 bForceMono;
    UInt32 noSubSamples;
    Int32 usb;
    Int32 lastUsb;

    Int32 bPsDataAvail;

    UInt32 bEnableIid;
    UInt32 bEnableIcc;

    UInt32 bEnableExt;
    Int32 bFineIidQ;

    Int32 aIidPrevFrameIndex[NO_HI_RES_BINS];
    Int32 aIccPrevFrameIndex[NO_HI_RES_BINS];

    UInt32 freqResIid;
    UInt32 freqResIcc;

    UInt32 bFrameClass;
    UInt32 noEnv;
    UInt32 aEnvStartStop[MAX_NO_PS_ENV+1];

    UInt32 abIidDtFlag[MAX_NO_PS_ENV];
    UInt32 abIccDtFlag[MAX_NO_PS_ENV];

    Int32   delayBufIndex;

    UInt32 aDelayRBufIndexSer[NO_SERIAL_ALLPASS_LINKS];

    Int32 **aaaRealDelayRBufferSerQmf[NO_SERIAL_ALLPASS_LINKS];
    Int32 **aaaImagDelayRBufferSerQmf[NO_SERIAL_ALLPASS_LINKS];

    Int32 **aaaRealDelayRBufferSerSubQmf[NO_SERIAL_ALLPASS_LINKS];
    Int32 **aaaImagDelayRBufferSerSubQmf[NO_SERIAL_ALLPASS_LINKS];

    Int32 **aaRealDelayBufferQmf;
    Int32 **aaImagDelayBufferQmf;
    Int32 **aaRealDelayBufferSubQmf;
    Int32 **aaImagDelayBufferSubQmf;

    Int32 *aPeakDecayFast;
    Int32 *aPrevNrg;
    Int32 *aPrevPeakDiff;

    Int32 *mHybridRealLeft;
    Int32 *mHybridImagLeft;
    Int32 *mHybridRealRight;
    Int32 *mHybridImagRight;

    HYBRID *hHybrid;

    Int32 h11Prev[NO_IID_GROUPS];
    Int32 h12Prev[NO_IID_GROUPS];
    Int32 h21Prev[NO_IID_GROUPS];
    Int32 h22Prev[NO_IID_GROUPS];

    Int32 H11[NO_IID_GROUPS];
    Int32 H12[NO_IID_GROUPS];
    Int32 H21[NO_IID_GROUPS];
    Int32 H22[NO_IID_GROUPS];

    Int32 deltaH11[NO_IID_GROUPS];
    Int32 deltaH12[NO_IID_GROUPS];
    Int32 deltaH21[NO_IID_GROUPS];
    Int32 deltaH22[NO_IID_GROUPS];

    Int32(*qmfBufferReal)[64];
    Int32(*qmfBufferImag)[64];

    Int32 aDelayBufIndex[NO_DELAY_CHANNELS];
    Int32 aNoSampleDelay[NO_DELAY_CHANNELS];
    Int32 aaIidIndex[MAX_NO_PS_ENV+1][NO_HI_RES_BINS];
    Int32 aaIccIndex[MAX_NO_PS_ENV+1][NO_HI_RES_BINS];

};

typedef struct PS_DEC STRUCT_PS_DEC;
typedef struct PS_DEC * HANDLE_PS_DEC;

#endif

