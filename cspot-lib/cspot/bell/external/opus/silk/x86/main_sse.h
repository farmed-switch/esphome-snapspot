

#ifndef MAIN_SSE_H
#define MAIN_SSE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

# if defined(OPUS_X86_MAY_HAVE_SSE4_1)

#if 0
#  define OVERRIDE_silk_VQ_WMat_EC

void silk_VQ_WMat_EC_sse4_1(
    opus_int8                   *ind,
    opus_int32                  *rate_dist_Q14,
    opus_int                    *gain_Q7,
    const opus_int16            *in_Q14,
    const opus_int32            *W_Q18,
    const opus_int8             *cb_Q7,
    const opus_uint8            *cb_gain_Q7,
    const opus_uint8            *cl_Q5,
    const opus_int              mu_Q9,
    const opus_int32            max_gain_Q7,
    opus_int                    L
);

#if defined OPUS_X86_PRESUME_SSE4_1

#define silk_VQ_WMat_EC(ind, rate_dist_Q14, gain_Q7, in_Q14, W_Q18, cb_Q7, cb_gain_Q7, cl_Q5, \
                          mu_Q9, max_gain_Q7, L, arch) \
    ((void)(arch),silk_VQ_WMat_EC_sse4_1(ind, rate_dist_Q14, gain_Q7, in_Q14, W_Q18, cb_Q7, cb_gain_Q7, cl_Q5, \
                          mu_Q9, max_gain_Q7, L))

#else

extern void (*const SILK_VQ_WMAT_EC_IMPL[OPUS_ARCHMASK + 1])(
    opus_int8                   *ind,
    opus_int32                  *rate_dist_Q14,
    opus_int                    *gain_Q7,
    const opus_int16            *in_Q14,
    const opus_int32            *W_Q18,
    const opus_int8             *cb_Q7,
    const opus_uint8            *cb_gain_Q7,
    const opus_uint8            *cl_Q5,
    const opus_int              mu_Q9,
    const opus_int32            max_gain_Q7,
    opus_int                    L
);

#  define silk_VQ_WMat_EC(ind, rate_dist_Q14, gain_Q7, in_Q14, W_Q18, cb_Q7, cb_gain_Q7, cl_Q5, \
                          mu_Q9, max_gain_Q7, L, arch) \
    ((*SILK_VQ_WMAT_EC_IMPL[(arch) & OPUS_ARCHMASK])(ind, rate_dist_Q14, gain_Q7, in_Q14, W_Q18, cb_Q7, cb_gain_Q7, cl_Q5, \
                          mu_Q9, max_gain_Q7, L))

#endif
#endif

#if 0
#  define OVERRIDE_silk_NSQ

void silk_NSQ_sse4_1(
    const silk_encoder_state    *psEncC,
    silk_nsq_state              *NSQ,
    SideInfoIndices             *psIndices,
    const opus_int32            x_Q3[],
    opus_int8                   pulses[],
    const opus_int16            PredCoef_Q12[ 2 * MAX_LPC_ORDER ],
    const opus_int16            LTPCoef_Q14[ LTP_ORDER * MAX_NB_SUBFR ],
    const opus_int16            AR2_Q13[ MAX_NB_SUBFR * MAX_SHAPE_LPC_ORDER ],
    const opus_int              HarmShapeGain_Q14[ MAX_NB_SUBFR ],
    const opus_int              Tilt_Q14[ MAX_NB_SUBFR ],
    const opus_int32            LF_shp_Q14[ MAX_NB_SUBFR ],
    const opus_int32            Gains_Q16[ MAX_NB_SUBFR ],
    const opus_int              pitchL[ MAX_NB_SUBFR ],
    const opus_int              Lambda_Q10,
    const opus_int              LTP_scale_Q14
);

#if defined OPUS_X86_PRESUME_SSE4_1

#define silk_NSQ(psEncC, NSQ, psIndices, x_Q3, pulses, PredCoef_Q12, LTPCoef_Q14, AR2_Q13, \
                   HarmShapeGain_Q14, Tilt_Q14, LF_shp_Q14, Gains_Q16, pitchL, Lambda_Q10, LTP_scale_Q14, arch) \
    ((void)(arch),silk_NSQ_sse4_1(psEncC, NSQ, psIndices, x_Q3, pulses, PredCoef_Q12, LTPCoef_Q14, AR2_Q13, \
                   HarmShapeGain_Q14, Tilt_Q14, LF_shp_Q14, Gains_Q16, pitchL, Lambda_Q10, LTP_scale_Q14))

#else

extern void (*const SILK_NSQ_IMPL[OPUS_ARCHMASK + 1])(
    const silk_encoder_state    *psEncC,
    silk_nsq_state              *NSQ,
    SideInfoIndices             *psIndices,
    const opus_int32            x_Q3[],
    opus_int8                   pulses[],
    const opus_int16            PredCoef_Q12[ 2 * MAX_LPC_ORDER ],
    const opus_int16            LTPCoef_Q14[ LTP_ORDER * MAX_NB_SUBFR ],
    const opus_int16            AR2_Q13[ MAX_NB_SUBFR * MAX_SHAPE_LPC_ORDER ],
    const opus_int              HarmShapeGain_Q14[ MAX_NB_SUBFR ],
    const opus_int              Tilt_Q14[ MAX_NB_SUBFR ],
    const opus_int32            LF_shp_Q14[ MAX_NB_SUBFR ],
    const opus_int32            Gains_Q16[ MAX_NB_SUBFR ],
    const opus_int              pitchL[ MAX_NB_SUBFR ],
    const opus_int              Lambda_Q10,
    const opus_int              LTP_scale_Q14
);

#  define silk_NSQ(psEncC, NSQ, psIndices, x_Q3, pulses, PredCoef_Q12, LTPCoef_Q14, AR2_Q13, \
                   HarmShapeGain_Q14, Tilt_Q14, LF_shp_Q14, Gains_Q16, pitchL, Lambda_Q10, LTP_scale_Q14, arch) \
    ((*SILK_NSQ_IMPL[(arch) & OPUS_ARCHMASK])(psEncC, NSQ, psIndices, x_Q3, pulses, PredCoef_Q12, LTPCoef_Q14, AR2_Q13, \
                   HarmShapeGain_Q14, Tilt_Q14, LF_shp_Q14, Gains_Q16, pitchL, Lambda_Q10, LTP_scale_Q14))

#endif

#  define OVERRIDE_silk_NSQ_del_dec

void silk_NSQ_del_dec_sse4_1(
    const silk_encoder_state    *psEncC,
    silk_nsq_state              *NSQ,
    SideInfoIndices             *psIndices,
    const opus_int32            x_Q3[],
    opus_int8                   pulses[],
    const opus_int16            PredCoef_Q12[ 2 * MAX_LPC_ORDER ],
    const opus_int16            LTPCoef_Q14[ LTP_ORDER * MAX_NB_SUBFR ],
    const opus_int16            AR2_Q13[ MAX_NB_SUBFR * MAX_SHAPE_LPC_ORDER ],
    const opus_int              HarmShapeGain_Q14[ MAX_NB_SUBFR ],
    const opus_int              Tilt_Q14[ MAX_NB_SUBFR ],
    const opus_int32            LF_shp_Q14[ MAX_NB_SUBFR ],
    const opus_int32            Gains_Q16[ MAX_NB_SUBFR ],
    const opus_int              pitchL[ MAX_NB_SUBFR ],
    const opus_int              Lambda_Q10,
    const opus_int              LTP_scale_Q14
);

#if defined OPUS_X86_PRESUME_SSE4_1

#define silk_NSQ_del_dec(psEncC, NSQ, psIndices, x_Q3, pulses, PredCoef_Q12, LTPCoef_Q14, AR2_Q13, \
                           HarmShapeGain_Q14, Tilt_Q14, LF_shp_Q14, Gains_Q16, pitchL, Lambda_Q10, LTP_scale_Q14, arch) \
    ((void)(arch),silk_NSQ_del_dec_sse4_1(psEncC, NSQ, psIndices, x_Q3, pulses, PredCoef_Q12, LTPCoef_Q14, AR2_Q13, \
                           HarmShapeGain_Q14, Tilt_Q14, LF_shp_Q14, Gains_Q16, pitchL, Lambda_Q10, LTP_scale_Q14))

#else

extern void (*const SILK_NSQ_DEL_DEC_IMPL[OPUS_ARCHMASK + 1])(
    const silk_encoder_state    *psEncC,
    silk_nsq_state              *NSQ,
    SideInfoIndices             *psIndices,
    const opus_int32            x_Q3[],
    opus_int8                   pulses[],
    const opus_int16            PredCoef_Q12[ 2 * MAX_LPC_ORDER ],
    const opus_int16            LTPCoef_Q14[ LTP_ORDER * MAX_NB_SUBFR ],
    const opus_int16            AR2_Q13[ MAX_NB_SUBFR * MAX_SHAPE_LPC_ORDER ],
    const opus_int              HarmShapeGain_Q14[ MAX_NB_SUBFR ],
    const opus_int              Tilt_Q14[ MAX_NB_SUBFR ],
    const opus_int32            LF_shp_Q14[ MAX_NB_SUBFR ],
    const opus_int32            Gains_Q16[ MAX_NB_SUBFR ],
    const opus_int              pitchL[ MAX_NB_SUBFR ],
    const opus_int              Lambda_Q10,
    const opus_int              LTP_scale_Q14
);

#  define silk_NSQ_del_dec(psEncC, NSQ, psIndices, x_Q3, pulses, PredCoef_Q12, LTPCoef_Q14, AR2_Q13, \
                           HarmShapeGain_Q14, Tilt_Q14, LF_shp_Q14, Gains_Q16, pitchL, Lambda_Q10, LTP_scale_Q14, arch) \
    ((*SILK_NSQ_DEL_DEC_IMPL[(arch) & OPUS_ARCHMASK])(psEncC, NSQ, psIndices, x_Q3, pulses, PredCoef_Q12, LTPCoef_Q14, AR2_Q13, \
                           HarmShapeGain_Q14, Tilt_Q14, LF_shp_Q14, Gains_Q16, pitchL, Lambda_Q10, LTP_scale_Q14))

#endif
#endif

void silk_noise_shape_quantizer(
    silk_nsq_state      *NSQ,
    opus_int            signalType,
    const opus_int32    x_sc_Q10[],
    opus_int8           pulses[],
    opus_int16          xq[],
    opus_int32          sLTP_Q15[],
    const opus_int16    a_Q12[],
    const opus_int16    b_Q14[],
    const opus_int16    AR_shp_Q13[],
    opus_int            lag,
    opus_int32          HarmShapeFIRPacked_Q14,
    opus_int            Tilt_Q14,
    opus_int32          LF_shp_Q14,
    opus_int32          Gain_Q16,
    opus_int            Lambda_Q10,
    opus_int            offset_Q10,
    opus_int            length,
    opus_int            shapingLPCOrder,
    opus_int            predictLPCOrder,
    int                 arch
);

void silk_VAD_GetNoiseLevels(
    const opus_int32            pX[ VAD_N_BANDS ],
    silk_VAD_state              *psSilk_VAD
);

#  define OVERRIDE_silk_VAD_GetSA_Q8

opus_int silk_VAD_GetSA_Q8_sse4_1(
    silk_encoder_state *psEnC,
    const opus_int16   pIn[]
);

#if defined(OPUS_X86_PRESUME_SSE4_1)
#define silk_VAD_GetSA_Q8(psEnC, pIn, arch) ((void)(arch),silk_VAD_GetSA_Q8_sse4_1(psEnC, pIn))

#else

#  define silk_VAD_GetSA_Q8(psEnC, pIn, arch) \
     ((*SILK_VAD_GETSA_Q8_IMPL[(arch) & OPUS_ARCHMASK])(psEnC, pIn))

extern opus_int (*const SILK_VAD_GETSA_Q8_IMPL[OPUS_ARCHMASK + 1])(
     silk_encoder_state *psEnC,
     const opus_int16   pIn[]);

#endif

# endif
#endif
