

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include "main_FIX.h"
#include "stack_alloc.h"
#include "tuning_parameters.h"

static OPUS_INLINE void silk_LBRR_encode_FIX(
    silk_encoder_state_FIX          *psEnc,
    silk_encoder_control_FIX        *psEncCtrl,
    const opus_int16                x16[],
    opus_int                        condCoding
);

void silk_encode_do_VAD_FIX(
    silk_encoder_state_FIX          *psEnc,
    opus_int                        activity
)
{
    const opus_int activity_threshold = SILK_FIX_CONST( SPEECH_ACTIVITY_DTX_THRES, 8 );

    silk_VAD_GetSA_Q8( &psEnc->sCmn, psEnc->sCmn.inputBuf + 1, psEnc->sCmn.arch );

    if( activity == VAD_NO_ACTIVITY && psEnc->sCmn.speech_activity_Q8 >= activity_threshold ) {
        psEnc->sCmn.speech_activity_Q8 = activity_threshold - 1;
    }

    if( psEnc->sCmn.speech_activity_Q8 < activity_threshold ) {
        psEnc->sCmn.indices.signalType = TYPE_NO_VOICE_ACTIVITY;
        psEnc->sCmn.noSpeechCounter++;
        if( psEnc->sCmn.noSpeechCounter <= NB_SPEECH_FRAMES_BEFORE_DTX ) {
            psEnc->sCmn.inDTX = 0;
        } else if( psEnc->sCmn.noSpeechCounter > MAX_CONSECUTIVE_DTX + NB_SPEECH_FRAMES_BEFORE_DTX ) {
            psEnc->sCmn.noSpeechCounter = NB_SPEECH_FRAMES_BEFORE_DTX;
            psEnc->sCmn.inDTX           = 0;
        }
        psEnc->sCmn.VAD_flags[ psEnc->sCmn.nFramesEncoded ] = 0;
    } else {
        psEnc->sCmn.noSpeechCounter    = 0;
        psEnc->sCmn.inDTX              = 0;
        psEnc->sCmn.indices.signalType = TYPE_UNVOICED;
        psEnc->sCmn.VAD_flags[ psEnc->sCmn.nFramesEncoded ] = 1;
    }
}

opus_int silk_encode_frame_FIX(
    silk_encoder_state_FIX          *psEnc,
    opus_int32                      *pnBytesOut,
    ec_enc                          *psRangeEnc,
    opus_int                        condCoding,
    opus_int                        maxBits,
    opus_int                        useCBR
)
{
    silk_encoder_control_FIX sEncCtrl;
    opus_int     i, iter, maxIter, found_upper, found_lower, ret = 0;
    opus_int16   *x_frame;
    ec_enc       sRangeEnc_copy, sRangeEnc_copy2;
    silk_nsq_state sNSQ_copy, sNSQ_copy2;
    opus_int32   seed_copy, nBits, nBits_lower, nBits_upper, gainMult_lower, gainMult_upper;
    opus_int32   gainsID, gainsID_lower, gainsID_upper;
    opus_int16   gainMult_Q8;
    opus_int16   ec_prevLagIndex_copy;
    opus_int     ec_prevSignalType_copy;
    opus_int8    LastGainIndex_copy2;
    opus_int     gain_lock[ MAX_NB_SUBFR ] = {0};
    opus_int16   best_gain_mult[ MAX_NB_SUBFR ];
    opus_int     best_sum[ MAX_NB_SUBFR ];
    SAVE_STACK;

    LastGainIndex_copy2 = nBits_lower = nBits_upper = gainMult_lower = gainMult_upper = 0;

    psEnc->sCmn.indices.Seed = psEnc->sCmn.frameCounter++ & 3;

    x_frame = psEnc->x_buf + psEnc->sCmn.ltp_mem_length;

    silk_LP_variable_cutoff( &psEnc->sCmn.sLP, psEnc->sCmn.inputBuf + 1, psEnc->sCmn.frame_length );

    silk_memcpy( x_frame + LA_SHAPE_MS * psEnc->sCmn.fs_kHz, psEnc->sCmn.inputBuf + 1, psEnc->sCmn.frame_length * sizeof( opus_int16 ) );

    if( !psEnc->sCmn.prefillFlag ) {
        VARDECL( opus_int16, res_pitch );
        VARDECL( opus_uint8, ec_buf_copy );
        opus_int16 *res_pitch_frame;

        ALLOC( res_pitch,
               psEnc->sCmn.la_pitch + psEnc->sCmn.frame_length
                   + psEnc->sCmn.ltp_mem_length, opus_int16 );

        res_pitch_frame = res_pitch + psEnc->sCmn.ltp_mem_length;

        silk_find_pitch_lags_FIX( psEnc, &sEncCtrl, res_pitch, x_frame - psEnc->sCmn.ltp_mem_length, psEnc->sCmn.arch );

        silk_noise_shape_analysis_FIX( psEnc, &sEncCtrl, res_pitch_frame, x_frame, psEnc->sCmn.arch );

        silk_find_pred_coefs_FIX( psEnc, &sEncCtrl, res_pitch_frame, x_frame, condCoding );

        silk_process_gains_FIX( psEnc, &sEncCtrl, condCoding );

        silk_LBRR_encode_FIX( psEnc, &sEncCtrl, x_frame, condCoding );

        maxIter = 6;
        gainMult_Q8 = SILK_FIX_CONST( 1, 8 );
        found_lower = 0;
        found_upper = 0;
        gainsID = silk_gains_ID( psEnc->sCmn.indices.GainsIndices, psEnc->sCmn.nb_subfr );
        gainsID_lower = -1;
        gainsID_upper = -1;

        silk_memcpy( &sRangeEnc_copy, psRangeEnc, sizeof( ec_enc ) );
        silk_memcpy( &sNSQ_copy, &psEnc->sCmn.sNSQ, sizeof( silk_nsq_state ) );
        seed_copy = psEnc->sCmn.indices.Seed;
        ec_prevLagIndex_copy = psEnc->sCmn.ec_prevLagIndex;
        ec_prevSignalType_copy = psEnc->sCmn.ec_prevSignalType;
        ALLOC( ec_buf_copy, 1275, opus_uint8 );
        for( iter = 0; ; iter++ ) {
            if( gainsID == gainsID_lower ) {
                nBits = nBits_lower;
            } else if( gainsID == gainsID_upper ) {
                nBits = nBits_upper;
            } else {

                if( iter > 0 ) {
                    silk_memcpy( psRangeEnc, &sRangeEnc_copy, sizeof( ec_enc ) );
                    silk_memcpy( &psEnc->sCmn.sNSQ, &sNSQ_copy, sizeof( silk_nsq_state ) );
                    psEnc->sCmn.indices.Seed = seed_copy;
                    psEnc->sCmn.ec_prevLagIndex = ec_prevLagIndex_copy;
                    psEnc->sCmn.ec_prevSignalType = ec_prevSignalType_copy;
                }

                if( psEnc->sCmn.nStatesDelayedDecision > 1 || psEnc->sCmn.warping_Q16 > 0 ) {
                    silk_NSQ_del_dec( &psEnc->sCmn, &psEnc->sCmn.sNSQ, &psEnc->sCmn.indices, x_frame, psEnc->sCmn.pulses,
                           sEncCtrl.PredCoef_Q12[ 0 ], sEncCtrl.LTPCoef_Q14, sEncCtrl.AR_Q13, sEncCtrl.HarmShapeGain_Q14,
                           sEncCtrl.Tilt_Q14, sEncCtrl.LF_shp_Q14, sEncCtrl.Gains_Q16, sEncCtrl.pitchL, sEncCtrl.Lambda_Q10, sEncCtrl.LTP_scale_Q14,
                           psEnc->sCmn.arch );
                } else {
                    silk_NSQ( &psEnc->sCmn, &psEnc->sCmn.sNSQ, &psEnc->sCmn.indices, x_frame, psEnc->sCmn.pulses,
                            sEncCtrl.PredCoef_Q12[ 0 ], sEncCtrl.LTPCoef_Q14, sEncCtrl.AR_Q13, sEncCtrl.HarmShapeGain_Q14,
                            sEncCtrl.Tilt_Q14, sEncCtrl.LF_shp_Q14, sEncCtrl.Gains_Q16, sEncCtrl.pitchL, sEncCtrl.Lambda_Q10, sEncCtrl.LTP_scale_Q14,
                            psEnc->sCmn.arch);
                }

                if ( iter == maxIter && !found_lower ) {
                    silk_memcpy( &sRangeEnc_copy2, psRangeEnc, sizeof( ec_enc ) );
                }

                silk_encode_indices( &psEnc->sCmn, psRangeEnc, psEnc->sCmn.nFramesEncoded, 0, condCoding );

                silk_encode_pulses( psRangeEnc, psEnc->sCmn.indices.signalType, psEnc->sCmn.indices.quantOffsetType,
                    psEnc->sCmn.pulses, psEnc->sCmn.frame_length );

                nBits = ec_tell( psRangeEnc );

                if ( iter == maxIter && !found_lower && nBits > maxBits ) {
                    silk_memcpy( psRangeEnc, &sRangeEnc_copy2, sizeof( ec_enc ) );

                    psEnc->sShape.LastGainIndex = sEncCtrl.lastGainIndexPrev;
                    for ( i = 0; i < psEnc->sCmn.nb_subfr; i++ ) {
                        psEnc->sCmn.indices.GainsIndices[ i ] = 4;
                    }
                    if (condCoding != CODE_CONDITIONALLY) {
                       psEnc->sCmn.indices.GainsIndices[ 0 ] = sEncCtrl.lastGainIndexPrev;
                    }
                    psEnc->sCmn.ec_prevLagIndex = ec_prevLagIndex_copy;
                    psEnc->sCmn.ec_prevSignalType = ec_prevSignalType_copy;

                    for ( i = 0; i < psEnc->sCmn.frame_length; i++ ) {
                        psEnc->sCmn.pulses[ i ] = 0;
                    }

                    silk_encode_indices( &psEnc->sCmn, psRangeEnc, psEnc->sCmn.nFramesEncoded, 0, condCoding );

                    silk_encode_pulses( psRangeEnc, psEnc->sCmn.indices.signalType, psEnc->sCmn.indices.quantOffsetType,
                        psEnc->sCmn.pulses, psEnc->sCmn.frame_length );

                    nBits = ec_tell( psRangeEnc );
                }

                if( useCBR == 0 && iter == 0 && nBits <= maxBits ) {
                    break;
                }
            }

            if( iter == maxIter ) {
                if( found_lower && ( gainsID == gainsID_lower || nBits > maxBits ) ) {

                    silk_memcpy( psRangeEnc, &sRangeEnc_copy2, sizeof( ec_enc ) );
                    celt_assert( sRangeEnc_copy2.offs <= 1275 );
                    silk_memcpy( psRangeEnc->buf, ec_buf_copy, sRangeEnc_copy2.offs );
                    silk_memcpy( &psEnc->sCmn.sNSQ, &sNSQ_copy2, sizeof( silk_nsq_state ) );
                    psEnc->sShape.LastGainIndex = LastGainIndex_copy2;
                }
                break;
            }

            if( nBits > maxBits ) {
                if( found_lower == 0 && iter >= 2 ) {

                    sEncCtrl.Lambda_Q10 = silk_ADD_RSHIFT32( sEncCtrl.Lambda_Q10, sEncCtrl.Lambda_Q10, 1 );
                    found_upper = 0;
                    gainsID_upper = -1;
                } else {
                    found_upper = 1;
                    nBits_upper = nBits;
                    gainMult_upper = gainMult_Q8;
                    gainsID_upper = gainsID;
                }
            } else if( nBits < maxBits - 5 ) {
                found_lower = 1;
                nBits_lower = nBits;
                gainMult_lower = gainMult_Q8;
                if( gainsID != gainsID_lower ) {
                    gainsID_lower = gainsID;

                    silk_memcpy( &sRangeEnc_copy2, psRangeEnc, sizeof( ec_enc ) );
                    celt_assert( psRangeEnc->offs <= 1275 );
                    silk_memcpy( ec_buf_copy, psRangeEnc->buf, psRangeEnc->offs );
                    silk_memcpy( &sNSQ_copy2, &psEnc->sCmn.sNSQ, sizeof( silk_nsq_state ) );
                    LastGainIndex_copy2 = psEnc->sShape.LastGainIndex;
                }
            } else {

                break;
            }

            if ( !found_lower && nBits > maxBits ) {
                int j;
                for ( i = 0; i < psEnc->sCmn.nb_subfr; i++ ) {
                    int sum=0;
                    for ( j = i*psEnc->sCmn.subfr_length; j < (i+1)*psEnc->sCmn.subfr_length; j++ ) {
                        sum += abs( psEnc->sCmn.pulses[j] );
                    }
                    if ( iter == 0 || (sum < best_sum[i] && !gain_lock[i]) ) {
                        best_sum[i] = sum;
                        best_gain_mult[i] = gainMult_Q8;
                    } else {
                        gain_lock[i] = 1;
                    }
                }
            }
            if( ( found_lower & found_upper ) == 0 ) {

                if( nBits > maxBits ) {
                    if (gainMult_Q8 < 16384) {
                        gainMult_Q8 *= 2;
                    } else {
                        gainMult_Q8 = 32767;
                    }
                } else {
                    opus_int32 gain_factor_Q16;
                    gain_factor_Q16 = silk_log2lin( silk_LSHIFT( nBits - maxBits, 7 ) / psEnc->sCmn.frame_length + SILK_FIX_CONST( 16, 7 ) );
                    gainMult_Q8 = silk_SMULWB( gain_factor_Q16, gainMult_Q8 );
                }

            } else {

                gainMult_Q8 = gainMult_lower + silk_DIV32_16( silk_MUL( gainMult_upper - gainMult_lower, maxBits - nBits_lower ), nBits_upper - nBits_lower );

                if( gainMult_Q8 > silk_ADD_RSHIFT32( gainMult_lower, gainMult_upper - gainMult_lower, 2 ) ) {
                    gainMult_Q8 = silk_ADD_RSHIFT32( gainMult_lower, gainMult_upper - gainMult_lower, 2 );
                } else
                if( gainMult_Q8 < silk_SUB_RSHIFT32( gainMult_upper, gainMult_upper - gainMult_lower, 2 ) ) {
                    gainMult_Q8 = silk_SUB_RSHIFT32( gainMult_upper, gainMult_upper - gainMult_lower, 2 );
                }
            }

            for( i = 0; i < psEnc->sCmn.nb_subfr; i++ ) {
                opus_int16 tmp;
                if ( gain_lock[i] ) {
                    tmp = best_gain_mult[i];
                } else {
                    tmp = gainMult_Q8;
                }
                sEncCtrl.Gains_Q16[ i ] = silk_LSHIFT_SAT32( silk_SMULWB( sEncCtrl.GainsUnq_Q16[ i ], tmp ), 8 );
            }

            psEnc->sShape.LastGainIndex = sEncCtrl.lastGainIndexPrev;
            silk_gains_quant( psEnc->sCmn.indices.GainsIndices, sEncCtrl.Gains_Q16,
                  &psEnc->sShape.LastGainIndex, condCoding == CODE_CONDITIONALLY, psEnc->sCmn.nb_subfr );

            gainsID = silk_gains_ID( psEnc->sCmn.indices.GainsIndices, psEnc->sCmn.nb_subfr );
        }
    }

    silk_memmove( psEnc->x_buf, &psEnc->x_buf[ psEnc->sCmn.frame_length ],
        ( psEnc->sCmn.ltp_mem_length + LA_SHAPE_MS * psEnc->sCmn.fs_kHz ) * sizeof( opus_int16 ) );

    if( psEnc->sCmn.prefillFlag ) {

        *pnBytesOut = 0;
        RESTORE_STACK;
        return ret;
    }

    psEnc->sCmn.prevLag        = sEncCtrl.pitchL[ psEnc->sCmn.nb_subfr - 1 ];
    psEnc->sCmn.prevSignalType = psEnc->sCmn.indices.signalType;

    psEnc->sCmn.first_frame_after_reset = 0;

    *pnBytesOut = silk_RSHIFT( ec_tell( psRangeEnc ) + 7, 3 );

    RESTORE_STACK;
    return ret;
}

static OPUS_INLINE void silk_LBRR_encode_FIX(
    silk_encoder_state_FIX          *psEnc,
    silk_encoder_control_FIX        *psEncCtrl,
    const opus_int16                x16[],
    opus_int                        condCoding
)
{
    opus_int32   TempGains_Q16[ MAX_NB_SUBFR ];
    SideInfoIndices *psIndices_LBRR = &psEnc->sCmn.indices_LBRR[ psEnc->sCmn.nFramesEncoded ];
    silk_nsq_state sNSQ_LBRR;

    if( psEnc->sCmn.LBRR_enabled && psEnc->sCmn.speech_activity_Q8 > SILK_FIX_CONST( LBRR_SPEECH_ACTIVITY_THRES, 8 ) ) {
        psEnc->sCmn.LBRR_flags[ psEnc->sCmn.nFramesEncoded ] = 1;

        silk_memcpy( &sNSQ_LBRR, &psEnc->sCmn.sNSQ, sizeof( silk_nsq_state ) );
        silk_memcpy( psIndices_LBRR, &psEnc->sCmn.indices, sizeof( SideInfoIndices ) );

        silk_memcpy( TempGains_Q16, psEncCtrl->Gains_Q16, psEnc->sCmn.nb_subfr * sizeof( opus_int32 ) );

        if( psEnc->sCmn.nFramesEncoded == 0 || psEnc->sCmn.LBRR_flags[ psEnc->sCmn.nFramesEncoded - 1 ] == 0 ) {

            psEnc->sCmn.LBRRprevLastGainIndex = psEnc->sShape.LastGainIndex;

            psIndices_LBRR->GainsIndices[ 0 ] = psIndices_LBRR->GainsIndices[ 0 ] + psEnc->sCmn.LBRR_GainIncreases;
            psIndices_LBRR->GainsIndices[ 0 ] = silk_min_int( psIndices_LBRR->GainsIndices[ 0 ], N_LEVELS_QGAIN - 1 );
        }

        silk_gains_dequant( psEncCtrl->Gains_Q16, psIndices_LBRR->GainsIndices,
            &psEnc->sCmn.LBRRprevLastGainIndex, condCoding == CODE_CONDITIONALLY, psEnc->sCmn.nb_subfr );

        if( psEnc->sCmn.nStatesDelayedDecision > 1 || psEnc->sCmn.warping_Q16 > 0 ) {
            silk_NSQ_del_dec( &psEnc->sCmn, &sNSQ_LBRR, psIndices_LBRR, x16,
                psEnc->sCmn.pulses_LBRR[ psEnc->sCmn.nFramesEncoded ], psEncCtrl->PredCoef_Q12[ 0 ], psEncCtrl->LTPCoef_Q14,
                psEncCtrl->AR_Q13, psEncCtrl->HarmShapeGain_Q14, psEncCtrl->Tilt_Q14, psEncCtrl->LF_shp_Q14,
                psEncCtrl->Gains_Q16, psEncCtrl->pitchL, psEncCtrl->Lambda_Q10, psEncCtrl->LTP_scale_Q14, psEnc->sCmn.arch );
        } else {
            silk_NSQ( &psEnc->sCmn, &sNSQ_LBRR, psIndices_LBRR, x16,
                psEnc->sCmn.pulses_LBRR[ psEnc->sCmn.nFramesEncoded ], psEncCtrl->PredCoef_Q12[ 0 ], psEncCtrl->LTPCoef_Q14,
                psEncCtrl->AR_Q13, psEncCtrl->HarmShapeGain_Q14, psEncCtrl->Tilt_Q14, psEncCtrl->LF_shp_Q14,
                psEncCtrl->Gains_Q16, psEncCtrl->pitchL, psEncCtrl->Lambda_Q10, psEncCtrl->LTP_scale_Q14, psEnc->sCmn.arch );
        }

        silk_memcpy( psEncCtrl->Gains_Q16, TempGains_Q16, psEnc->sCmn.nb_subfr * sizeof( opus_int32 ) );
    }
}
