

#ifndef SILK_STRUCTS_FLP_H
#define SILK_STRUCTS_FLP_H

#include "typedef.h"
#include "main.h"
#include "structs.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
    opus_int8                   LastGainIndex;
    silk_float                  HarmShapeGain_smth;
    silk_float                  Tilt_smth;
} silk_shape_state_FLP;

typedef struct {
    silk_encoder_state          sCmn;
    silk_shape_state_FLP        sShape;

    silk_float                  x_buf[ 2 * MAX_FRAME_LENGTH + LA_SHAPE_MAX ];
    silk_float                  LTPCorr;
} silk_encoder_state_FLP;

typedef struct {

    silk_float                  Gains[ MAX_NB_SUBFR ];
    silk_float                  PredCoef[ 2 ][ MAX_LPC_ORDER ];
    silk_float                  LTPCoef[LTP_ORDER * MAX_NB_SUBFR];
    silk_float                  LTP_scale;
    opus_int                    pitchL[ MAX_NB_SUBFR ];

    silk_float                  AR[ MAX_NB_SUBFR * MAX_SHAPE_LPC_ORDER ];
    silk_float                  LF_MA_shp[     MAX_NB_SUBFR ];
    silk_float                  LF_AR_shp[     MAX_NB_SUBFR ];
    silk_float                  Tilt[          MAX_NB_SUBFR ];
    silk_float                  HarmShapeGain[ MAX_NB_SUBFR ];
    silk_float                  Lambda;
    silk_float                  input_quality;
    silk_float                  coding_quality;

    silk_float                  predGain;
    silk_float                  LTPredCodGain;
    silk_float                  ResNrg[ MAX_NB_SUBFR ];

    opus_int32                  GainsUnq_Q16[ MAX_NB_SUBFR ];
    opus_int8                   lastGainIndexPrev;
} silk_encoder_control_FLP;

typedef struct {
    silk_encoder_state_FLP      state_Fxx[ ENCODER_NUM_CHANNELS ];
    stereo_enc_state            sStereo;
    opus_int32                  nBitsUsedLBRR;
    opus_int32                  nBitsExceeded;
    opus_int                    nChannelsAPI;
    opus_int                    nChannelsInternal;
    opus_int                    nPrevChannelsInternal;
    opus_int                    timeSinceSwitchAllowed_ms;
    opus_int                    allowBandwidthSwitch;
    opus_int                    prev_decode_only_middle;
} silk_encoder;

#ifdef __cplusplus
}
#endif

#endif
