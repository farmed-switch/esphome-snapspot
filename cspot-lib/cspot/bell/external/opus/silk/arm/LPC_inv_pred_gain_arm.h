

#ifndef SILK_LPC_INV_PRED_GAIN_ARM_H
# define SILK_LPC_INV_PRED_GAIN_ARM_H

# include "celt/arm/armcpu.h"

# if defined(OPUS_ARM_MAY_HAVE_NEON_INTR)
opus_int32 silk_LPC_inverse_pred_gain_neon(
    const opus_int16            *A_Q12,
    const opus_int              order
);

#  if !defined(OPUS_HAVE_RTCD) && defined(OPUS_ARM_PRESUME_NEON)
#   define OVERRIDE_silk_LPC_inverse_pred_gain            (1)
#   define silk_LPC_inverse_pred_gain(A_Q12, order, arch) ((void)(arch), PRESUME_NEON(silk_LPC_inverse_pred_gain)(A_Q12, order))
#  endif
# endif

# if !defined(OVERRIDE_silk_LPC_inverse_pred_gain)

#  if defined(OPUS_HAVE_RTCD) && (defined(OPUS_ARM_MAY_HAVE_NEON_INTR) && !defined(OPUS_ARM_PRESUME_NEON_INTR))
extern opus_int32 (*const SILK_LPC_INVERSE_PRED_GAIN_IMPL[OPUS_ARCHMASK+1])(const opus_int16 *A_Q12, const opus_int order);
#   define OVERRIDE_silk_LPC_inverse_pred_gain            (1)
#   define silk_LPC_inverse_pred_gain(A_Q12, order, arch) ((*SILK_LPC_INVERSE_PRED_GAIN_IMPL[(arch)&OPUS_ARCHMASK])(A_Q12, order))
#  elif defined(OPUS_ARM_PRESUME_NEON_INTR)
#   define OVERRIDE_silk_LPC_inverse_pred_gain            (1)
#   define silk_LPC_inverse_pred_gain(A_Q12, order, arch) ((void)(arch), silk_LPC_inverse_pred_gain_neon(A_Q12, order))
#  endif
# endif

#endif
