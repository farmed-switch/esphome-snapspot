

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "celt/x86/x86cpu.h"
#include "structs.h"
#include "SigProc_FIX.h"
#include "pitch.h"
#include "main.h"

#if !defined(OPUS_X86_PRESUME_SSE4_1)

#if defined(FIXED_POINT)

#include "fixed/main_FIX.h"

opus_int64 (*const SILK_INNER_PROD16_ALIGNED_64_IMPL[ OPUS_ARCHMASK + 1 ] )(
    const opus_int16 *inVec1,
    const opus_int16 *inVec2,
    const opus_int   len
) = {
  silk_inner_prod16_aligned_64_c,
  silk_inner_prod16_aligned_64_c,
  silk_inner_prod16_aligned_64_c,
  MAY_HAVE_SSE4_1( silk_inner_prod16_aligned_64 ),
  MAY_HAVE_SSE4_1( silk_inner_prod16_aligned_64 )
};

#endif

opus_int (*const SILK_VAD_GETSA_Q8_IMPL[ OPUS_ARCHMASK + 1 ] )(
    silk_encoder_state *psEncC,
    const opus_int16   pIn[]
) = {
  silk_VAD_GetSA_Q8_c,
  silk_VAD_GetSA_Q8_c,
  silk_VAD_GetSA_Q8_c,
  MAY_HAVE_SSE4_1( silk_VAD_GetSA_Q8 ),
  MAY_HAVE_SSE4_1( silk_VAD_GetSA_Q8 )
};

#if 0
void (*const SILK_NSQ_IMPL[ OPUS_ARCHMASK + 1 ] )(
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
) = {
  silk_NSQ_c,
  silk_NSQ_c,
  silk_NSQ_c,
  MAY_HAVE_SSE4_1( silk_NSQ ),
  MAY_HAVE_SSE4_1( silk_NSQ )
};
#endif

#if 0
void (*const SILK_VQ_WMAT_EC_IMPL[ OPUS_ARCHMASK + 1 ] )(
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
) = {
  silk_VQ_WMat_EC_c,
  silk_VQ_WMat_EC_c,
  silk_VQ_WMat_EC_c,
  MAY_HAVE_SSE4_1( silk_VQ_WMat_EC ),
  MAY_HAVE_SSE4_1( silk_VQ_WMat_EC )
};
#endif

#if 0
void (*const SILK_NSQ_DEL_DEC_IMPL[ OPUS_ARCHMASK + 1 ] )(
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
) = {
  silk_NSQ_del_dec_c,
  silk_NSQ_del_dec_c,
  silk_NSQ_del_dec_c,
  MAY_HAVE_SSE4_1( silk_NSQ_del_dec ),
  MAY_HAVE_SSE4_1( silk_NSQ_del_dec )
};
#endif

#if defined(FIXED_POINT)

void (*const SILK_BURG_MODIFIED_IMPL[ OPUS_ARCHMASK + 1 ] )(
    opus_int32                  *res_nrg,
    opus_int                    *res_nrg_Q,
    opus_int32                  A_Q16[],
    const opus_int16            x[],
    const opus_int32            minInvGain_Q30,
    const opus_int              subfr_length,
    const opus_int              nb_subfr,
    const opus_int              D,
    int                         arch
) = {
  silk_burg_modified_c,
  silk_burg_modified_c,
  silk_burg_modified_c,
  MAY_HAVE_SSE4_1( silk_burg_modified ),
  MAY_HAVE_SSE4_1( silk_burg_modified )
};

#endif
#endif
