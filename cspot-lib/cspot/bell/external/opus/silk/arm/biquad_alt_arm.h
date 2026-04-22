

#ifndef SILK_BIQUAD_ALT_ARM_H
# define SILK_BIQUAD_ALT_ARM_H

# include "celt/arm/armcpu.h"

# if defined(OPUS_ARM_MAY_HAVE_NEON_INTR)
void silk_biquad_alt_stride2_neon(
    const opus_int16            *in,
    const opus_int32            *B_Q28,
    const opus_int32            *A_Q28,
    opus_int32                  *S,
    opus_int16                  *out,
    const opus_int32            len
);

#  if !defined(OPUS_HAVE_RTCD) && defined(OPUS_ARM_PRESUME_NEON)
#   define OVERRIDE_silk_biquad_alt_stride2                   (1)
#   define silk_biquad_alt_stride2(in, B_Q28, A_Q28, S, out, len, arch) ((void)(arch), PRESUME_NEON(silk_biquad_alt_stride2)(in, B_Q28, A_Q28, S, out, len))
#  endif
# endif

# if !defined(OVERRIDE_silk_biquad_alt_stride2)

#  if defined(OPUS_HAVE_RTCD) && (defined(OPUS_ARM_MAY_HAVE_NEON_INTR) && !defined(OPUS_ARM_PRESUME_NEON_INTR))
extern void (*const SILK_BIQUAD_ALT_STRIDE2_IMPL[OPUS_ARCHMASK+1])(
        const opus_int16            *in,
        const opus_int32            *B_Q28,
        const opus_int32            *A_Q28,
        opus_int32                  *S,
        opus_int16                  *out,
        const opus_int32            len
    );
#   define OVERRIDE_silk_biquad_alt_stride2                  (1)
#   define silk_biquad_alt_stride2(in, B_Q28, A_Q28, S, out, len, arch) ((*SILK_BIQUAD_ALT_STRIDE2_IMPL[(arch)&OPUS_ARCHMASK])(in, B_Q28, A_Q28, S, out, len))
#  elif defined(OPUS_ARM_PRESUME_NEON_INTR)
#   define OVERRIDE_silk_biquad_alt_stride2                  (1)
#   define silk_biquad_alt_stride2(in, B_Q28, A_Q28, S, out, len, arch) ((void)(arch), silk_biquad_alt_stride2_neon(in, B_Q28, A_Q28, S, out, len))
#  endif
# endif

#endif
