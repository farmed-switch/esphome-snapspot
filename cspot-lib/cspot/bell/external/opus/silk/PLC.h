

#ifndef SILK_PLC_H
#define SILK_PLC_H

#include "main.h"

#define BWE_COEF                        0.99
#define V_PITCH_GAIN_START_MIN_Q14      11469
#define V_PITCH_GAIN_START_MAX_Q14      15565
#define MAX_PITCH_LAG_MS                18
#define RAND_BUF_SIZE                   128
#define RAND_BUF_MASK                   ( RAND_BUF_SIZE - 1 )
#define LOG2_INV_LPC_GAIN_HIGH_THRES    3
#define LOG2_INV_LPC_GAIN_LOW_THRES     8
#define PITCH_DRIFT_FAC_Q16             655

void silk_PLC_Reset(
    silk_decoder_state                  *psDec
);

void silk_PLC(
    silk_decoder_state                  *psDec,
    silk_decoder_control                *psDecCtrl,
    opus_int16                          frame[],
    opus_int                            lost,
    int                                 arch
);

void silk_PLC_glue_frames(
    silk_decoder_state                  *psDec,
    opus_int16                          frame[],
    opus_int                            length
);

#endif

