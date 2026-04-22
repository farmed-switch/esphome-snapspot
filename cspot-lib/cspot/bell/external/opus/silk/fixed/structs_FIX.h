

#ifndef SILK_STRUCTS_FIX_H
#define SILK_STRUCTS_FIX_H

#include "typedef.h"
#include "main.h"
#include "structs.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
    opus_int8                   LastGainIndex;
    opus_int32                  HarmBoost_smth_Q16;
    opus_int32                  HarmShapeGain_smth_Q16;
    opus_int32                  Tilt_smth_Q16;
} silk_shape_state_FIX;

typedef struct {
    silk_encoder_state          sCmn;
    silk_shape_state_FIX        sShape;

    silk_DWORD_ALIGN opus_int16 x_buf[ 2 * MAX_FRAME_LENGTH + LA_SHAPE_MAX ];
    opus_int                    LTPCorr_Q15;
    opus_int32                    resNrgSmth;
} silk_encoder_state_FIX;

typedef struct {

    opus_int32                  Gains_Q16[ MAX_NB_SUBFR ];
    silk_DWORD_ALIGN opus_int16 PredCoef_Q12[ 2 ][ MAX_LPC_ORDER ];
    opus_int16                  LTPCoef_Q14[ LTP_ORDER * MAX_NB_SUBFR ];
    opus_int                    LTP_scale_Q14;
    opus_int                    pitchL[ MAX_NB_SUBFR ];

    silk_DWORD_ALIGN opus_int16 AR_Q13[ MAX_NB_SUBFR * MAX_SHAPE_LPC_ORDER ];
    opus_int32                  LF_shp_Q14[        MAX_NB_SUBFR ];
    opus_int                    Tilt_Q14[          MAX_NB_SUBFR ];
    opus_int                    HarmShapeGain_Q14[ MAX_NB_SUBFR ];
    opus_int                    Lambda_Q10;
    opus_int                    input_quality_Q14;
    opus_int                    coding_quality_Q14;

    opus_int32                  predGain_Q16;
    opus_int                    LTPredCodGain_Q7;
    opus_int32                  ResNrg[ MAX_NB_SUBFR ];
    opus_int                    ResNrgQ[ MAX_NB_SUBFR ];

    opus_int32                  GainsUnq_Q16[ MAX_NB_SUBFR ];
    opus_int8                   lastGainIndexPrev;
} silk_encoder_control_FIX;

typedef struct {
    silk_encoder_state_FIX      state_Fxx[ ENCODER_NUM_CHANNELS ];
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
