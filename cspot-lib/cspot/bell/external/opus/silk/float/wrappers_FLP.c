

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main_FLP.h"

void silk_A2NLSF_FLP(
    opus_int16                      *NLSF_Q15,
    const silk_float                *pAR,
    const opus_int                  LPC_order
)
{
    opus_int   i;
    opus_int32 a_fix_Q16[ MAX_LPC_ORDER ];

    for( i = 0; i < LPC_order; i++ ) {
        a_fix_Q16[ i ] = silk_float2int( pAR[ i ] * 65536.0f );
    }

    silk_A2NLSF( NLSF_Q15, a_fix_Q16, LPC_order );
}

void silk_NLSF2A_FLP(
    silk_float                      *pAR,
    const opus_int16                *NLSF_Q15,
    const opus_int                  LPC_order,
    int                             arch
)
{
    opus_int   i;
    opus_int16 a_fix_Q12[ MAX_LPC_ORDER ];

    silk_NLSF2A( a_fix_Q12, NLSF_Q15, LPC_order, arch );

    for( i = 0; i < LPC_order; i++ ) {
        pAR[ i ] = ( silk_float )a_fix_Q12[ i ] * ( 1.0f / 4096.0f );
    }
}

void silk_process_NLSFs_FLP(
    silk_encoder_state              *psEncC,
    silk_float                      PredCoef[ 2 ][ MAX_LPC_ORDER ],
    opus_int16                      NLSF_Q15[      MAX_LPC_ORDER ],
    const opus_int16                prev_NLSF_Q15[ MAX_LPC_ORDER ]
)
{
    opus_int     i, j;
    opus_int16   PredCoef_Q12[ 2 ][ MAX_LPC_ORDER ];

    silk_process_NLSFs( psEncC, PredCoef_Q12, NLSF_Q15, prev_NLSF_Q15);

    for( j = 0; j < 2; j++ ) {
        for( i = 0; i < psEncC->predictLPCOrder; i++ ) {
            PredCoef[ j ][ i ] = ( silk_float )PredCoef_Q12[ j ][ i ] * ( 1.0f / 4096.0f );
        }
    }
}

void silk_NSQ_wrapper_FLP(
    silk_encoder_state_FLP          *psEnc,
    silk_encoder_control_FLP        *psEncCtrl,
    SideInfoIndices                 *psIndices,
    silk_nsq_state                  *psNSQ,
    opus_int8                       pulses[],
    const silk_float                x[]
)
{
    opus_int     i, j;
    opus_int16   x16[ MAX_FRAME_LENGTH ];
    opus_int32   Gains_Q16[ MAX_NB_SUBFR ];
    silk_DWORD_ALIGN opus_int16 PredCoef_Q12[ 2 ][ MAX_LPC_ORDER ];
    opus_int16   LTPCoef_Q14[ LTP_ORDER * MAX_NB_SUBFR ];
    opus_int     LTP_scale_Q14;

    opus_int16   AR_Q13[ MAX_NB_SUBFR * MAX_SHAPE_LPC_ORDER ];
    opus_int32   LF_shp_Q14[ MAX_NB_SUBFR ];
    opus_int     Lambda_Q10;
    opus_int     Tilt_Q14[ MAX_NB_SUBFR ];
    opus_int     HarmShapeGain_Q14[ MAX_NB_SUBFR ];

    for( i = 0; i < psEnc->sCmn.nb_subfr; i++ ) {
        for( j = 0; j < psEnc->sCmn.shapingLPCOrder; j++ ) {
            AR_Q13[ i * MAX_SHAPE_LPC_ORDER + j ] = silk_float2int( psEncCtrl->AR[ i * MAX_SHAPE_LPC_ORDER + j ] * 8192.0f );
        }
    }

    for( i = 0; i < psEnc->sCmn.nb_subfr; i++ ) {
        LF_shp_Q14[ i ] =   silk_LSHIFT32( silk_float2int( psEncCtrl->LF_AR_shp[ i ]     * 16384.0f ), 16 ) |
                              (opus_uint16)silk_float2int( psEncCtrl->LF_MA_shp[ i ]     * 16384.0f );
        Tilt_Q14[ i ]   =        (opus_int)silk_float2int( psEncCtrl->Tilt[ i ]          * 16384.0f );
        HarmShapeGain_Q14[ i ] = (opus_int)silk_float2int( psEncCtrl->HarmShapeGain[ i ] * 16384.0f );
    }
    Lambda_Q10 = ( opus_int )silk_float2int( psEncCtrl->Lambda * 1024.0f );

    for( i = 0; i < psEnc->sCmn.nb_subfr * LTP_ORDER; i++ ) {
        LTPCoef_Q14[ i ] = (opus_int16)silk_float2int( psEncCtrl->LTPCoef[ i ] * 16384.0f );
    }

    for( j = 0; j < 2; j++ ) {
        for( i = 0; i < psEnc->sCmn.predictLPCOrder; i++ ) {
            PredCoef_Q12[ j ][ i ] = (opus_int16)silk_float2int( psEncCtrl->PredCoef[ j ][ i ] * 4096.0f );
        }
    }

    for( i = 0; i < psEnc->sCmn.nb_subfr; i++ ) {
        Gains_Q16[ i ] = silk_float2int( psEncCtrl->Gains[ i ] * 65536.0f );
        silk_assert( Gains_Q16[ i ] > 0 );
    }

    if( psIndices->signalType == TYPE_VOICED ) {
        LTP_scale_Q14 = silk_LTPScales_table_Q14[ psIndices->LTP_scaleIndex ];
    } else {
        LTP_scale_Q14 = 0;
    }

    for( i = 0; i < psEnc->sCmn.frame_length; i++ ) {
        x16[ i ] = silk_float2int( x[ i ] );
    }

    if( psEnc->sCmn.nStatesDelayedDecision > 1 || psEnc->sCmn.warping_Q16 > 0 ) {
        silk_NSQ_del_dec( &psEnc->sCmn, psNSQ, psIndices, x16, pulses, PredCoef_Q12[ 0 ], LTPCoef_Q14,
            AR_Q13, HarmShapeGain_Q14, Tilt_Q14, LF_shp_Q14, Gains_Q16, psEncCtrl->pitchL, Lambda_Q10, LTP_scale_Q14, psEnc->sCmn.arch );
    } else {
        silk_NSQ( &psEnc->sCmn, psNSQ, psIndices, x16, pulses, PredCoef_Q12[ 0 ], LTPCoef_Q14,
            AR_Q13, HarmShapeGain_Q14, Tilt_Q14, LF_shp_Q14, Gains_Q16, psEncCtrl->pitchL, Lambda_Q10, LTP_scale_Q14, psEnc->sCmn.arch );
    }
}

void silk_quant_LTP_gains_FLP(
    silk_float                      B[ MAX_NB_SUBFR * LTP_ORDER ],
    opus_int8                       cbk_index[ MAX_NB_SUBFR ],
    opus_int8                       *periodicity_index,
    opus_int32                      *sum_log_gain_Q7,
    silk_float                      *pred_gain_dB,
    const silk_float                XX[ MAX_NB_SUBFR * LTP_ORDER * LTP_ORDER ],
    const silk_float                xX[ MAX_NB_SUBFR * LTP_ORDER ],
    const opus_int                    subfr_len,
    const opus_int                    nb_subfr,
    int                             arch
)
{
    opus_int   i, pred_gain_dB_Q7;
    opus_int16 B_Q14[ MAX_NB_SUBFR * LTP_ORDER ];
    opus_int32 XX_Q17[ MAX_NB_SUBFR * LTP_ORDER * LTP_ORDER ];
    opus_int32 xX_Q17[ MAX_NB_SUBFR * LTP_ORDER ];

    for( i = 0; i < nb_subfr * LTP_ORDER * LTP_ORDER; i++ ) {
        XX_Q17[ i ] = (opus_int32)silk_float2int( XX[ i ] * 131072.0f );
    }
    for( i = 0; i < nb_subfr * LTP_ORDER; i++ ) {
        xX_Q17[ i ] = (opus_int32)silk_float2int( xX[ i ] * 131072.0f );
    }

    silk_quant_LTP_gains( B_Q14, cbk_index, periodicity_index, sum_log_gain_Q7, &pred_gain_dB_Q7, XX_Q17, xX_Q17, subfr_len, nb_subfr, arch );

    for( i = 0; i < nb_subfr * LTP_ORDER; i++ ) {
        B[ i ] = (silk_float)B_Q14[ i ] * ( 1.0f / 16384.0f );
    }

    *pred_gain_dB = (silk_float)pred_gain_dB_Q7 * ( 1.0f / 128.0f );
}
