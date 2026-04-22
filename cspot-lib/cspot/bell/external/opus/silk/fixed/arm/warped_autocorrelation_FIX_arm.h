

#ifndef SILK_WARPED_AUTOCORRELATION_FIX_ARM_H
# define SILK_WARPED_AUTOCORRELATION_FIX_ARM_H

# include "celt/arm/armcpu.h"

# if defined(FIXED_POINT)

#  if defined(OPUS_ARM_MAY_HAVE_NEON_INTR)
void silk_warped_autocorrelation_FIX_neon(
          opus_int32                *corr,
          opus_int                  *scale,
    const opus_int16                *input,
    const opus_int                  warping_Q16,
    const opus_int                  length,
    const opus_int                  order
);

#  if !defined(OPUS_HAVE_RTCD) && defined(OPUS_ARM_PRESUME_NEON)
#   define OVERRIDE_silk_warped_autocorrelation_FIX (1)
#   define silk_warped_autocorrelation_FIX(corr, scale, input, warping_Q16, length, order, arch) \
    ((void)(arch), PRESUME_NEON(silk_warped_autocorrelation_FIX)(corr, scale, input, warping_Q16, length, order))
#  endif
#  endif

#  if !defined(OVERRIDE_silk_warped_autocorrelation_FIX)

#   if defined(OPUS_HAVE_RTCD) && (defined(OPUS_ARM_MAY_HAVE_NEON_INTR) && !defined(OPUS_ARM_PRESUME_NEON_INTR))
extern void (*const SILK_WARPED_AUTOCORRELATION_FIX_IMPL[OPUS_ARCHMASK+1])(opus_int32*, opus_int*, const opus_int16*, const opus_int, const opus_int, const opus_int);
#    define OVERRIDE_silk_warped_autocorrelation_FIX (1)
#    define silk_warped_autocorrelation_FIX(corr, scale, input, warping_Q16, length, order, arch) \
    ((*SILK_WARPED_AUTOCORRELATION_FIX_IMPL[(arch)&OPUS_ARCHMASK])(corr, scale, input, warping_Q16, length, order))
#   elif defined(OPUS_ARM_PRESUME_NEON_INTR)
#    define OVERRIDE_silk_warped_autocorrelation_FIX (1)
#    define silk_warped_autocorrelation_FIX(corr, scale, input, warping_Q16, length, order, arch) \
    ((void)(arch), silk_warped_autocorrelation_FIX_neon(corr, scale, input, warping_Q16, length, order))
#   endif
#  endif

# endif

#endif
