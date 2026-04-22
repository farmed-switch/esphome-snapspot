
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "main_FIX.h"
#include "NSQ.h"
#include "SigProc_FIX.h"

#if defined(OPUS_HAVE_RTCD)

# if (defined(OPUS_ARM_MAY_HAVE_NEON_INTR) && \
 !defined(OPUS_ARM_PRESUME_NEON_INTR))

void (*const SILK_BIQUAD_ALT_STRIDE2_IMPL[OPUS_ARCHMASK + 1])(
        const opus_int16            *in,
        const opus_int32            *B_Q28,
        const opus_int32            *A_Q28,
        opus_int32                  *S,
        opus_int16                  *out,
        const opus_int32            len
) = {
      silk_biquad_alt_stride2_c,
      silk_biquad_alt_stride2_c,
      silk_biquad_alt_stride2_c,
      silk_biquad_alt_stride2_neon,
};

opus_int32 (*const SILK_LPC_INVERSE_PRED_GAIN_IMPL[OPUS_ARCHMASK + 1])(
        const opus_int16            *A_Q12,
        const opus_int              order
) = {
      silk_LPC_inverse_pred_gain_c,
      silk_LPC_inverse_pred_gain_c,
      silk_LPC_inverse_pred_gain_c,
      silk_LPC_inverse_pred_gain_neon,
};

void  (*const SILK_NSQ_DEL_DEC_IMPL[OPUS_ARCHMASK + 1])(
        const silk_encoder_state    *psEncC,
        silk_nsq_state              *NSQ,
        SideInfoIndices             *psIndices,
        const opus_int16            x16[],
        opus_int8                   pulses[],
        const opus_int16            PredCoef_Q12[ 2 * MAX_LPC_ORDER ],
        const opus_int16            LTPCoef_Q14[ LTP_ORDER * MAX_NB_SUBFR ],
        const opus_int16            AR_Q13[ MAX_NB_SUBFR * MAX_SHAPE_LPC_ORDER ],
        const opus_int              HarmShapeGain_Q14[ MAX_NB_SUBFR ],
        const opus_int              Tilt_Q14[ MAX_NB_SUBFR ],
        const opus_int32            LF_shp_Q14[ MAX_NB_SUBFR ],
        const opus_int32            Gains_Q16[ MAX_NB_SUBFR ],
        const opus_int              pitchL[ MAX_NB_SUBFR ],
        const opus_int              Lambda_Q10,
        const opus_int              LTP_scale_Q14
) = {
      silk_NSQ_del_dec_c,
      silk_NSQ_del_dec_c,
      silk_NSQ_del_dec_c,
      silk_NSQ_del_dec_neon,
};

opus_int32
 (*const SILK_NSQ_NOISE_SHAPE_FEEDBACK_LOOP_IMPL[OPUS_ARCHMASK+1])(
 const opus_int32 *data0, opus_int32 *data1, const opus_int16 *coef,
 opus_int order) = {
  silk_NSQ_noise_shape_feedback_loop_c,
  silk_NSQ_noise_shape_feedback_loop_c,
  silk_NSQ_noise_shape_feedback_loop_c,
  silk_NSQ_noise_shape_feedback_loop_neon,
};

# endif

# if defined(FIXED_POINT) && \
 defined(OPUS_ARM_MAY_HAVE_NEON_INTR) && !defined(OPUS_ARM_PRESUME_NEON_INTR)

void (*const SILK_WARPED_AUTOCORRELATION_FIX_IMPL[OPUS_ARCHMASK + 1])(
          opus_int32                *corr,
          opus_int                  *scale,
    const opus_int16                *input,
    const opus_int                  warping_Q16,
    const opus_int                  length,
    const opus_int                  order
) = {
      silk_warped_autocorrelation_FIX_c,
      silk_warped_autocorrelation_FIX_c,
      silk_warped_autocorrelation_FIX_c,
      silk_warped_autocorrelation_FIX_neon,
};

# endif

#endif
