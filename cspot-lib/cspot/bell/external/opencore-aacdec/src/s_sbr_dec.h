

#ifndef S_SBR_DEC_H
#define S_SBR_DEC_H

#include    "s_sbr_frame_data.h"
#include    "pv_audio_type_defs.h"
#include    "s_patch.h"
#include    "e_blockswitching.h"

typedef struct
{
    Int32 outSampleRate;
    Int32 LC_aacP_DecoderFlag;

    Int32 startIndexCodecQmf;
    Int32 lowBandAddSamples;
    Int32 noCols;
    Int32 qmfBufLen;
    Int32 bufWriteOffs;
    Int32 bufReadOffs;

    Int32 sbStopCodec;
    Int   lowSubband;
    Int   prevLowSubband;
    Int32 highSubband;
    Int32 noSubbands;

    Int   FreqBandTable[2][MAX_FREQ_COEFFS + 1];
    Int32 FreqBandTableNoise[MAX_NOISE_COEFFS + 1];
    Int32 V_k_master[MAX_FREQ_COEFFS + 1];
    Int32 NSfb[2];
    Int32 NoNoiseBands;
    Int32 Num_Master;

    struct PATCH Patch;

    Int32 gateMode[4];
    Int32 limSbc[4][12 + 1];

    Int32 sqrt_cache[8][4];

} SBR_DEC;

#endif

