

#ifndef S_SBR_FRAME_DATA_H
#define S_SBR_FRAME_DATA_H

#include    "pv_audio_type_defs.h"
#include    "s_sbr_header_data.h"
#include    "e_invf_mode.h"
#include    "e_coupling_mode.h"
#include    "sbr_constants.h"
#include    "s_patch.h"
#include    "config.h"

typedef struct
{
    Int32 nScaleFactors;
    Int32 nNoiseFactors;
    Int32 crcCheckSum;
    Int32 frameClass;
    Int32 frameInfo[LENGTH_FRAME_INFO];
    Int32 nSfb[2];
    Int32 nNfb;
    Int32 offset;
    Int32 ampRes;
    Int32 nNoiseFloorEnvelopes;
    Int32 p;
    Int32 prevEnvIsShort;

    Int32 reset_flag;

    SBR_HEADER_DATA sbr_header;

    Int32 domain_vec1[MAX_ENVELOPES];
    Int32 domain_vec2[MAX_ENVELOPES];

    INVF_MODE sbr_invf_mode[MAX_NUM_NOISE_VALUES];
    INVF_MODE sbr_invf_mode_prev[MAX_NUM_NOISE_VALUES];

    COUPLING_MODE coupling;

    Int32 addHarmonics[MAX_NUM_ENVELOPE_VALUES];

    Int32 hFp[64];
    Int32 harm_index;
    Int32 phase_index;
    Int32 sUp;

    Int32 iEnvelope_man[MAX_NUM_ENVELOPE_VALUES];
    Int32 iEnvelope_exp[MAX_NUM_ENVELOPE_VALUES];
    Int32 sfb_nrg_prev_man[MAX_FREQ_COEFFS];

    Int32 sbrNoiseFloorLevel_man[MAX_NUM_NOISE_VALUES];
    Int32 sbrNoiseFloorLevel_exp[MAX_NUM_NOISE_VALUES];
    Int32 prevNoiseLevel_man[MAX_NUM_NOISE_VALUES];

    Int32  BwVector[MAX_NUM_PATCHES];
    Int32  BwVectorOld[MAX_NUM_PATCHES];

    Int32 codecQmfBufferReal[40][32];
    Int32 *sbrQmfBufferReal;
    Int32 HistsbrQmfBufferReal[6*SBR_NUM_BANDS];
#ifdef HQ_SBR
    Int32 codecQmfBufferImag[40][32];
    Int32 *sbrQmfBufferImag;
    Int32 HistsbrQmfBufferImag[6*SBR_NUM_BANDS];
#endif
    Int16  V[1152];

    Int32 degreeAlias[64];

#ifdef HQ_SBR

    Int32 fBuffer_man[5][64];
    Int32 fBufferN_man[5][64];
    Int32 fBuffer_exp[5][64];
    Int32 fBufferN_exp[5][64];

    Int32 *fBuf_man[64];
    Int32 *fBuf_exp[64];
    Int32 *fBufN_man[64];
    Int32 *fBufN_exp[64];

#endif

}
SBR_FRAME_DATA;

#endif

